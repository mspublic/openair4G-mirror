#ifndef __RLC_AM_SEGMENT_HOLES_H__
#    define __RLC_AM_SEGMENT_HOLES_H__
//-----------------------------------------------------------------------------
#    ifdef RLC_AM_SEGMENT_HOLES_C
#        define private_rlc_am_segments_holes(x)    x
#        define protected_rlc_am_segments_holes(x)  x
#        define public_rlc_am_segments_holes(x)     x
#    else
#        ifdef RLC_AM_MODULE
#            define private_rlc_am_segments_holes(x)
#            define protected_rlc_am_segments_holes(x)  extern x
#            define public_rlc_am_segments_holes(x)     extern x
#        else
#            define private_rlc_am_segments_holes(x)
#            define protected_rlc_am_segments_holes(x)
#            define public_rlc_am_segments_holes(x)     extern x
#        endif
#    endif
protected_rlc_am_segments_holes(void rlc_am_clear_holes (rlc_am_entity_t *rlcP, u16_t snP);)
protected_rlc_am_segments_holes(void rlc_am_remove_hole (rlc_am_entity_t *rlcP, u16_t snP, u16_t so_startP, u16_t so_stopP);)

protected_rlc_am_segments_holes(void rlc_am_get_next_hole (rlc_am_entity_t *rlcP, u16_t snP, int* so_startP, int* so_stopP);)
protected_rlc_am_segments_holes(void rlc_am_add_hole (rlc_am_entity_t *rlcP, u16_t snP, u16_t so_startP, u16_t so_stopP)
;)
#endif
