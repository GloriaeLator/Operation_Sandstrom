# OPERATION SANDSTORM

## Challenge 2

The given verify_corrupted.bin,the expectedd output is 1; but gives output 0; To find a one byte change to fix this. 

Observe thatit loops for few times from 0x50 to 0x70, but the value that is printed as output is the present in 0x8c

```
00000050: 0404 0100  ....  |
00000054: 0f03 0400  ....  |
00000058: 0306 0400  ....  |
0000005c: 0f01 0600  ....  |
00000060: 0306 0100  ....  |----> loop 
00000064: 0f02 0600  ....  |     
00000068: 0306 1000  ....  |
0000006c: 0602 0600  ....  |   
00000070: 0e00 5000  ..P.  |  
00000074: 0302 fc09  .... 
00000078: 0603 0200  ....  |----> comparison (not equal)
0000007c: 0300 0100  ....  |----> reg[0] = 1
00000080: 0301 0000  ....  |----> reg[0] = 0
00000084: 0200 8c00  ....  |----> comparison at 0x78 is not equal, so no movement
00000088: 0700 0100  ....  |----> re-assigns reg[0] = reg[1]
0000008c: 1000 0000  ....  |----> prints reg[0]
00000090: 0d00 0000  ....

BY END OF LOOP reg[03] = 0x09FD (from debugger)
reg[02] = 0x09FC BEFORE COMPARISON 

```

Hence it is obvious to change the `0x084: 0200 8C00` to `0x084: 0E00 8C00` 