#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "OctetString.h"

#ifndef TRANSACTION_IDENTIFIER_H_
#define TRANSACTION_IDENTIFIER_H_

#define TRANSACTION_IDENTIFIER_MINIMUM_LENGTH 1
#define TRANSACTION_IDENTIFIER_MAXIMUM_LENGTH 1

typedef struct {
    uint8_t field;
} TransactionIdentifier;

int encode_transaction_identifier(TransactionIdentifier *transactionidentifier, uint8_t iei, uint8_t *buffer, uint32_t len);

void dump_transaction_identifier_xml(TransactionIdentifier *transactionidentifier, uint8_t iei);

int decode_transaction_identifier(TransactionIdentifier *transactionidentifier, uint8_t iei, uint8_t *buffer, uint32_t len);

#endif /* TRANSACTION IDENTIFIER_H_ */

