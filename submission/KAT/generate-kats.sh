#!/bin/bash

NTSKEM1264DIR="../Reference_Implementation/kem/nts_kem_12_64"
NTSKEM1380DIR="../Reference_Implementation/kem/nts_kem_13_80"
NTSKEM13136DIR="../Reference_Implementation/kem/nts_kem_13_136"
NTSKEM1264INTVAL="$NTSKEM1264DIR/bin/ntskem-12-64-intval";
NTSKEM1380INTVAL="$NTSKEM1380DIR/bin/ntskem-13-80-intval";
NTSKEM13136INTVAL="$NTSKEM13136DIR/bin/ntskem-13-136-intval";

# Check for the existence of the required scripts
command -v $NTSKEM1264INTVAL > /dev/null 2>&1  || { echo >&2 "$NTSKEM1264INTVAL does not exist, creating the binary now."; cd $NTSKEM1264DIR && make && cd -; }
command -v $NTSKEM1380INTVAL > /dev/null 2>&1  || { echo >&2 "$NTSKEM1380INTVAL does not exist, creating the binary now."; cd $NTSKEM1380DIR && make && cd -; }
command -v $NTSKEM13136INTVAL > /dev/null 2>&1 || { echo >&2 "$NTSKEM13136INTVAL does not exist, creating the binary now."; cd $NTSKEM13136DIR && make && cd -; }

# Generating KATs for NTS-KEM(12,64)
echo "Generating KATs for NTS-KEM(12,64). Please note that this will take a few minutes to complete.";
cd kem/nts_kem_12_64
../../$NTSKEM1264INTVAL > PQCkemKAT_328736.intval
cd ../../

# Generating KATs for NTS-KEM(13,80)
echo "Generating KATs for NTS-KEM(13,80). Please note that this will also take a few minutes to complete.";
cd kem/nts_kem_13_80
../../$NTSKEM1380INTVAL > PQCkemKAT_947316.intval
cd ../../

#kem/:
# Generating KATs for NTS-KEM(13,136)
echo "Generating KATs for NTS-KEM(13,136). Please note that this will take longer than the two above to complete.";
cd kem/nts_kem_13_136
../../$NTSKEM13136INTVAL > PQCkemKAT_1439626.intval
cd ../../

