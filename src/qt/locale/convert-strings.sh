#!/bin/bash

sed -i "s/\bbitcoin\b/dallar/g" *.ts
sed -i "s/\bBitcoin\b/Dallar/g" *.ts
sed -i "s/\bBITCOINS\b/GROESTLCOINS/g" *.ts
sed -i "s/\bbitcoins\b/dallars/g" *.ts
sed -i "s/\bBitcoins/Dallars/g" *.ts

sed -i "s/ bitcoin/ dallar/g" *.ts
sed -i "s/ Bitcoin/ Dallar/g" *.ts
sed -i "s/ BITCOIN/ GROESTLCOIN/g" *.ts
