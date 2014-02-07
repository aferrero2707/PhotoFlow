#ifndef PF_VIPS_LAYER_H
#define PF_VIPS_LAYER_H

#ifdef __cplusplus
//extern "C" {
#endif /*__cplusplus*/

int
vips_layer( VipsImage **in, int n, VipsImage **out, int first, 
	    PF::ProcessorBase* proc,
	    VipsImage* imap, VipsImage* omap, 
	    VipsDemandStyle demand_hint);

#ifdef __cplusplus
//}
#endif /*__cplusplus*/


#endif
