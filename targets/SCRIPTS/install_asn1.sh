#!/bin/bash
which asn1c > /dev/null
if [ $? -eq 0 ] ; then echo "asn1c is installed" ; else echo "Please install asn1c (version 0.9.22 or greater)" ; fi
cd $OPENAIR2_DIR/RRC/LITE/MESSAGES && asn1c -gen-PER -fcompound-names -fnative-types -fskeletons-copy $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1c/ASN1_files/EUTRA-RRC-Definitions.asn

cat $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1c/ASN1_files/EUTRA-RRC-Definitions-a20.asn | grep   INTEGER\ \:\: | sed  's/INTEGER ::=//g' | sed 's/--/\/\//g' | sed 's/^/#define /' | sed 's/\-1/_minus_1/g' | sed 's/\-/_/g' > $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.tmp ; rm -f /tmp/EUTRA-RRC-Definitions-a20.tmp ; cat $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1c/ASN1_files/EUTRA-RRC-Definitions-a20.asn | grep  --invert-match SEQUENCE | grep INTEGER\ \( | uniq | grep \:\:\= | sed  's/INTEGER\ //g'  | sed 's/[()]//g' | tr "." " " | sed  's/\:\:\=//g' | tr '\t' " "  > /tmp/EUTRA-RRC-Definitions-a20.tmp ; cat /tmp/EUTRA-RRC-Definitions-a20.tmp | sed 's/^ *//g' | sed 's/ \{1,\}/ /g' | cut --complement -d ' '  -f3 | sed 's/^/#define min_val_/' | sed 's/\ -/\ +/g' | sed 's/-/_/g' | sed  's/\ +/\ -/g' >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.tmp ; cat /tmp/EUTRA-RRC-Definitions-a20.tmp | sed 's/^ *//g' | sed 's/ \{1,\}/ /g' | cut --complement -d ' '  -f2 | sed 's/^/#define max_val_/' | sed 's/\ -/\ +/g' | sed 's/-/_/g' | sed  's/\ +/\ -/g' >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.tmp;

echo "#ifndef __ASN1_A20_CONSTANTS_H__" > $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.h
echo "#define __ASN1_A20_CONSTANTS_H__" >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.h
cat $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.tmp >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.h
echo "#endif __ASN1_A20_CONSTANTS_H__" >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.h
rm -f $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-a20_constants.tmp;

cat $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1c/ASN1_files/EUTRA-RRC-Definitions-86.asn | grep   INTEGER\ \:\: | sed  's/INTEGER ::=//g' | sed 's/--/\/\//g' | sed 's/^/#define /' | sed 's/\-1/_minus_1/g' | sed 's/\-/_/g' > $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.tmp ; rm -f /tmp/EUTRA-RRC-Definitions-86.tmp ; cat $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1c/ASN1_files/EUTRA-RRC-Definitions-86.asn | grep  --invert-match SEQUENCE | grep INTEGER\ \( | uniq | grep \:\:\= | sed  's/INTEGER\ //g'  | sed 's/[()]//g' | tr "." " " | sed  's/\:\:\=//g' | tr '\t' " "  > /tmp/EUTRA-RRC-Definitions-86.tmp ; cat /tmp/EUTRA-RRC-Definitions-86.tmp | sed 's/^ *//g' | sed 's/ \{1,\}/ /g' | cut --complement -d ' '  -f3 | sed 's/^/#define min_val_/' | sed 's/\ -/\ +/g' | sed 's/-/_/g' | sed  's/\ +/\ -/g' >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.tmp ; cat /tmp/EUTRA-RRC-Definitions-86.tmp | sed 's/^ *//g' | sed 's/ \{1,\}/ /g' | cut --complement -d ' '  -f2 | sed 's/^/#define max_val_/' | sed 's/\ -/\ +/g' | sed 's/-/_/g' | sed  's/\ +/\ -/g' >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.tmp;

echo "#ifndef __ASN1_86_CONSTANTS_H__" > $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.h
echo "#define __ASN1_86_CONSTANTS_H__" >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.h
cat $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.tmp >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.h
echo "#endif __ASN1_86_CONSTANTS_H__" >> $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.h
rm -f $OPENAIR2_DIR/RRC/LITE/MESSAGES/asn1-86_constants.tmp;

cd $OPENAIR_TARGETS

