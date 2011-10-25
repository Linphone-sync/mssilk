/*
	NO LICENSE
*/

#include "SKP_Silk_SDK_API.h"
#include "mediastreamer2/mscodecutils.h"
#include "mediastreamer2/msticker.h"
#include "mediastreamer2/msfilter.h"


/* Define codec specific settings */
#define MAX_BYTES_PER_FRAME     250 // Equals peak bitrate of 100 kbps 
#define MAX_INPUT_FRAMES        5

/*filter common method*/
struct enc_struct {
	SKP_SILK_SDK_EncControlStruct control;
	void* psEnc;
	uint32_t ts;
	MSBufferizer *bufferizer;
	unsigned char ptime;
	unsigned char max_ptime;
	unsigned int max_network_bitrate;
};

static void enc_init(MSFilter *f){
    struct enc_struct* obj;
	f->data = ms_new0(struct enc_struct,1);
	obj = (struct enc_struct*) f->data;
	SKP_int16 ret;
	SKP_int32 encSizeBytes;
	
	/* Create encoder */
	ret = SKP_Silk_SDK_Get_Encoder_Size(&encSizeBytes );
	if( ret ) {
		ms_error("SKP_Silk_SDK_Get_Encoder_Size returned %i", ret );
	}
	obj->psEnc = ms_malloc(encSizeBytes);
	/* Reset decoder */
	ret = SKP_Silk_SDK_InitEncoder(obj->psEnc,&obj->control);
	if(ret) {
		ms_error( "SKP_Silk_SDK_InitEncoder returned %i", ret );
	}
	obj->ptime=20;
	obj->max_ptime=100;
	obj->bufferizer=ms_bufferizer_new();
	obj->control.useInBandFEC=1;
	obj->control.complexity=1;
	obj->control.packetLossPercentage=5;
}

static void enc_preprocess(MSFilter *f){
    
}

static void enc_process(MSFilter *f){
	mblk_t *im;
	mblk_t *om=NULL;
	SKP_int16 ret;
	SKP_int16 nBytes;
	uint8_t * buff=NULL;
	struct enc_struct* obj= (struct enc_struct*) f->data;
	obj->control.packetSize = obj->control.API_sampleRate*obj->ptime/1000; /*in sample*/
	
	while((im=ms_queue_get(f->inputs[0]))!=NULL){
		ms_bufferizer_put(obj->bufferizer,im);
	}
	while(ms_bufferizer_get_avail(obj->bufferizer)>=obj->control.packetSize*2){
		/* max payload size */
        nBytes = MAX_BYTES_PER_FRAME * MAX_INPUT_FRAMES;			
		om = allocb(nBytes,0);
		if (!buff) buff=ms_malloc(obj->control.packetSize*2);
		ms_bufferizer_read(obj->bufferizer,buff,obj->control.packetSize*2);
		ret = SKP_Silk_SDK_Encode(obj->psEnc
								  , &obj->control
								  , (const SKP_int16*)buff 
								  , (SKP_int16)(obj->control.packetSize)
								  , om->b_wptr
								  , &nBytes );
		if(ret) {
			ms_error( "SKP_Silk_Encode returned %i", ret );
			freeb(om);
		} else  if (nBytes > 0) {
			obj->ts+=obj->control.packetSize;
			om->b_wptr+=nBytes;	
			mblk_set_timestamp_info(om,obj->ts);
			ms_queue_put(f->outputs[0],om);
			om=NULL;
		} 
		
	}
	if (buff!=NULL) {
		ms_free(buff);
	}

}

static void enc_postprocess(MSFilter *f){
    
}

static void enc_unit(MSFilter *f){
    struct enc_struct* obj= (struct enc_struct*) f->data;
	ms_bufferizer_destroy(obj->bufferizer);
	ms_free(obj->psEnc);
	ms_free(f->data);
}


/*filter specific method*/

static int enc_set_sample_rate(MSFilter *f, void *arg) {
	struct enc_struct* obj= (struct enc_struct*) f->data;
	switch (*(SKP_int32*)arg) {
		case 8000:
		case 12000:
		case 16000:
		case 24000:
			obj->control.maxInternalSampleRate=*(SKP_int32*)arg;
			obj->control.API_sampleRate=*(SKP_int32*)arg;
			break;
		default:
			ms_warning("unsupported max sampling rate [%i] for silk, using 16 000",*(SKP_int32*)arg);
			obj->control.API_sampleRate=obj->control.maxInternalSampleRate=16000;
	}

	return 0;
}

static int enc_get_sample_rate(MSFilter *f, void *arg) {
	struct enc_struct* obj= (struct enc_struct*) f->data;
    *(int*)arg = obj->control.maxInternalSampleRate;
	return 0;
}

static int enc_add_fmtp(MSFilter *f, void *arg){
	char buf[64];
	struct enc_struct* obj= (struct enc_struct*) f->data;
	const char *fmtp=(const char *)arg;
	buf[0] ='\0';
	
	if (fmtp_get_value(fmtp,"maxptime:",buf,sizeof(buf))){
		obj->max_ptime=atoi(buf);
		if (obj->max_ptime <20 || obj->max_ptime >100 ) {
			ms_warning("MSSilkEnc unknown value [%i] for maxptime, use default value (100) instead",obj->max_ptime);
			obj->max_ptime=100;
		}
		ms_message("MSSilkEnc: got useinbandfec=%i",obj->max_ptime);
	} else 	if (fmtp_get_value(fmtp,"ptime",buf,sizeof(buf))){
		obj->ptime=atoi(buf);
		if (obj->ptime > obj->max_ptime) {
			obj->ptime=obj->max_ptime;
		} else if (obj->ptime%20) {
		//if the ptime is not a mulptiple of 20, go to the next multiple
		obj->ptime = obj->ptime - obj->ptime%20 + 20; 
		}
		
		ms_message("MSSilkEnc: got ptime=%i",obj->ptime);
	} else 	if (fmtp_get_value(fmtp,"useinbandfec",buf,sizeof(buf))){
		obj->control.useInBandFEC=atoi(buf);
		if (obj->control.useInBandFEC != 0 && obj->control.useInBandFEC != 1) {
			ms_warning("MSSilkEnc unknown value [%i] for useinbandfec, use default value (0) instead",obj->control.useInBandFEC);
			obj->control.useInBandFEC=1;
		}
		ms_message("MSSilkEnc: got useinbandfec=%i",obj->control.useInBandFEC);
	} 
	
	return 0;
}
static int enc_set_bitrate(MSFilter *f, void *arg){
	struct enc_struct* obj= (struct enc_struct*) f->data;
	int inital_cbr=0;
	int normalized_cbr=0;	
	int pps=1000/obj->ptime;
	obj->max_network_bitrate=*(int*)arg;
	normalized_cbr=inital_cbr=(int)( ((((float)obj->max_network_bitrate)/(pps*8))-20-12-8)*pps*8);
	switch(obj->control.maxInternalSampleRate) {
		case 8000:
			normalized_cbr=MIN(normalized_cbr,20000);
			normalized_cbr=MAX(normalized_cbr,5000);
			break;
		case 12000:
			normalized_cbr=MIN(normalized_cbr,25000);
			normalized_cbr=MAX(normalized_cbr,7000);
			break;
		case 16000:
			normalized_cbr=MIN(normalized_cbr,32000);
			normalized_cbr=MAX(normalized_cbr,8000);
			break;
		case 24000:
			normalized_cbr=MIN(normalized_cbr,40000);
			normalized_cbr=MAX(normalized_cbr,20000);
			break;
			
	}
	if (normalized_cbr!=inital_cbr) {
		ms_warning("Silk enc unsupported codec bitrate [%i], normalizing",inital_cbr); 
	}
	obj->control.bitRate=normalized_cbr;
	ms_message("Setting silk codec birate to [%i] from network bitrate [%i] with ptime [%i]",obj->control.bitRate,obj->max_network_bitrate,obj->ptime);
	return 0;
}

static int enc_get_bitrate(MSFilter *f, void *arg){
	struct enc_struct* obj= (struct enc_struct*) f->data;	
	*(int*)arg=obj->max_network_bitrate;
	return 0;
}


static MSFilterMethod enc_methods[]={
	{	MS_FILTER_SET_SAMPLE_RATE	,	enc_set_sample_rate },
	{	MS_FILTER_GET_SAMPLE_RATE	,	enc_get_sample_rate },
	{	MS_FILTER_SET_BITRATE		,	enc_set_bitrate	},
	{	MS_FILTER_GET_BITRATE		,	enc_get_bitrate	},
	{	MS_FILTER_ADD_FMTP		,	enc_add_fmtp },
	{	0, NULL}
};



MSFilterDesc ms_silk_enc_desc={
	.id=MS_FILTER_PLUGIN_ID, /* from Allfilters.h*/
	.name="MSSILKEnc",
	.text="SILK audio encoder filter.",
	.category=MS_FILTER_ENCODER,
	.enc_fmt="SILK",
	.ninputs=1, /*number of inputs*/
	.noutputs=1, /*number of outputs*/
	.init=enc_init,
	.preprocess=enc_preprocess,
	.process=enc_process,
	.postprocess=enc_postprocess,
	.uninit=enc_unit,
	.methods=enc_methods
};










/*filter common method*/
struct silk_dec_struct {
	SKP_SILK_SDK_DecControlStruct control;
	void  *psDec;
	MSConcealerContext *concealer;
	MSRtpPayloadPickerContext rtp_picker_context;
	unsigned  short int sequence_number;
	
};

static void dec_init(MSFilter *f){
        f->data = ms_new0(struct silk_dec_struct,1);
}

static void dec_preprocess(MSFilter *f){
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
	SKP_int16 ret;
	SKP_int32 decSizeBytes;
	/* Initialize to one frame per packet, for proper concealment before first packet arrives */
	obj->control.framesPerPacket = 1;
	/* Create decoder */
	ret = SKP_Silk_SDK_Get_Decoder_Size(&decSizeBytes );
	if( ret ) {
		ms_error("SKP_Silk_SDK_Get_Decoder_Size returned %d", ret );
	}
	obj->psDec = ms_malloc(decSizeBytes);
	/* Reset decoder */
	ret = SKP_Silk_SDK_InitDecoder(obj->psDec);
	if(ret) {
		ms_error( "SKP_Silk_InitDecoder returned %d", ret );
	}
	obj->concealer = ms_concealer_context_new(UINT32_MAX);
}
/**
 put im to NULL for PLC
 */

static void decode(MSFilter *f, mblk_t *im) {
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
	mblk_t *om;
	SKP_int16 len;
	SKP_int16 ret;
	/* Decode 20 ms */
	om=allocb(obj->control.API_sampleRate*4/100,0); /*samplingrate*0.02*2*/ 
	ret = SKP_Silk_SDK_Decode( obj->psDec, &obj->control, im?0:1, im?im->b_rptr:0, im?(im->b_wptr - im->b_rptr):0, (SKP_int16*)om->b_wptr, &len );
	if( ret ) {
		ms_error( "SKP_Silk_SDK_Decode returned %d", ret );
		ms_free(om);
	} else {
		
		om->b_wptr+=len*2;
		ms_queue_put(f->outputs[0],om);
	}
	if (im && ms_concealer_context_get_sampling_time(obj->concealer) == 0) {
		/*need to initialize the time*/
		ms_concealer_context_set_sampling_time(obj->concealer,f->ticker->time);
	}
	obj->sequence_number = im?mblk_get_cseq(im):++obj->sequence_number;
	
	ms_concealer_context_set_sampling_time(obj->concealer,(im?ms_concealer_context_get_sampling_time(obj->concealer):f->ticker->time)+20);
	
}
static void dec_process(MSFilter *f){
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
	mblk_t* im;
	mblk_t* fec_im;
	int i;
	SKP_int16 n_bytes_fec=0;
	
	while((im=ms_queue_get(f->inputs[0]))) {
		
		do {
			decode(f,im);
			/* Until last 20 ms frame of packet has been decoded */
		} while(obj->control.moreInternalDecoderFrames); 
	}
	
	if (ms_concealer_context_is_concealement_required(obj->concealer, f->ticker->time)) {
		//first try fec
		if (obj->rtp_picker_context.picker) {
			fec_im = allocb(obj->control.API_sampleRate*4/100,0);/*probbaly too big*/
			for (i=0;i<2;i++) {
				im = obj->rtp_picker_context.picker(&obj->rtp_picker_context,obj->sequence_number+i+1);
				if (im) {
					SKP_Silk_SDK_search_for_LBRR( im->b_rptr, im->b_wptr - im->b_rptr, i + 1, (SKP_uint8*)fec_im->b_wptr, &n_bytes_fec );
					if (n_bytes_fec>0) {
						ms_message("Silk dec, got fec from jitter buffer");
						fec_im->b_wptr+=n_bytes_fec;
						mblk_set_cseq(fec_im,obj->sequence_number+1);
						break;
					}
				}
			}
			if (n_bytes_fec ==0) {
				/*too bad no fec packet found*/
				freeb(fec_im);
				fec_im=NULL;
			}
		}
		
		decode(f,fec_im); /*ig fec_im == NULL, plc*/
	}
	
}

static void dec_postprocess(MSFilter *f){
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
	ms_message("SILK plc count=%li",ms_concealer_context_get_total_number_of_plc(obj->concealer));
	ms_free(obj->psDec);
	ms_concealer_context_destroy(obj->concealer);
}

static void dec_unit(MSFilter *f){
	ms_free(f->data);
}


/*filter specific method*/

static int dec_set_sample_rate(MSFilter *f, void *arg) {
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
	switch (*(SKP_int32*)arg) {
		case 8000:
		case 12000:
		case 16000:
		case 24000:
		case 32000:
		case 44000:
		case 48000:	
			obj->control.API_sampleRate=*(SKP_int32*)arg;
			break;
		default:
			ms_warning("Unsupported output sampling rate [%i] for silk, using 44 000",*(SKP_int32*)arg);
			obj->control.API_sampleRate=44000;
	}
	return 0;
}

static int dec_get_sample_rate(MSFilter *f, void *arg) {
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
    *(int*)arg = obj->control.API_sampleRate;
	return 0;
}
static int dec_set_rtp_picker(MSFilter *f, void *arg) {
	struct silk_dec_struct* obj= (struct silk_dec_struct*) f->data;
	obj->rtp_picker_context=*(MSRtpPayloadPickerContext*)arg;
	return 0;
}
static MSFilterMethod dec_methods[]={
	{	MS_FILTER_SET_SAMPLE_RATE , dec_set_sample_rate },
    {	MS_FILTER_GET_SAMPLE_RATE , dec_get_sample_rate },
	{	MS_FILTER_SET_RTP_PAYLOAD_PICKER,dec_set_rtp_picker},
	{	0, NULL}
};



MSFilterDesc ms_silk_dec_desc={
	.id=MS_FILTER_PLUGIN_ID, /* from Allfilters.h*/
	.name="MSSILKDec",
	.text="Silk decoder filter.",
	.category=MS_FILTER_DECODER,
	.enc_fmt="SILK",
	.ninputs=1, /*number of inputs*/
	.noutputs=1, /*number of outputs*/
	.init=dec_init,
	.preprocess=dec_preprocess,
	.process=dec_process,
	.postprocess=dec_postprocess,
	.uninit=dec_unit,
	.methods=dec_methods,
	.flags=MS_FILTER_IS_PUMP
};









#ifdef _MSC_VER
#define MS_PLUGIN_DECLARE(type) __declspec(dllexport) type
#else
#define MS_PLUGIN_DECLARE(type) type
#endif

MS_PLUGIN_DECLARE(void) libmssilk_init(){
	ms_filter_register(&ms_silk_enc_desc);
	ms_filter_register(&ms_silk_dec_desc);
}
