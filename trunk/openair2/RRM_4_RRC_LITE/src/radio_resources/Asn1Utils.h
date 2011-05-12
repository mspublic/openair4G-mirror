#ifndef _ASN1UTILS_H
#    define _ASN1UTILS_H

#    include "PhysicalConfigDedicated.h"
#    include "RadioResourceConfigDedicated.h"
#    include "DRB-ToAddMod.h"
#    include "SRB-ToAddMod.h"
#    include "MAC-MainConfig.h"

using namespace std;

class Asn1Utils  {
    public:

        Asn1Utils () {};
        ~Asn1Utils ();

        static DRB_ToAddMod_t* Clone(DRB_ToAddMod_t*);
        static SRB_ToAddMod_t* Clone(SRB_ToAddMod_t*);
};
#    endif

