Interaction sequence
Client sends c0+c1 to server Server sends s0+s1+s2 to client Client sends c2 to server

Note that some servers receive c2 before sending s2.

Length
c0 and s0 are both 1 byte c1 and c2 and s1 and s2 are both 1536 bytes.

Two modes
There are two modes of rtmp handshake: simple mode and complex mode.

The following table is the handshake modes used by some common rtmp client programs and software that I have compiled.

Name Mode
obs push stream simple
vlc player pull stream Complex
mpv player pull stream Complex
ffmpeg push/pull stream Complex
nginx-rtmp-module push/pull Complex
The rtmp server should normally support both modes. (Otherwise it would be awkward if rtmp clients using one of the modes could handshake and the other couldn't.)

This document mainly refers to the implementation of nginx-rtmp-module (hereafter referred to as nrm) to lal implementation. nrm, as an rtmp open source server, has a handshake implementation when acting as a server. It also has a handshake implementation as a client since it supports relaying. lal is my own streaming server written in Go, which supports the rtmp protocol. rtmp handshake implementation is written with reference to nrm. The internal rtmp client handshake uses simple mode. rtmp server handshake supports both modes. lal github address: https://github.com/q191201771/lal

This single byte for c0 and s0 is the version number, which is the same for both simple and complex modes. It is fixed to 0x03. c1 and c2 and s1 and s2 have different formats in both modes.

s1 can be seen as a reply to c1 and c2 as a reply to s2.

The nrm refers to c0c1 and s0s1 as CHALLENGE and c2 and s2 as RESPONSE.

Simple mode
See spec-rtmp_specification_1.0.pdf at https://github.com/q191201771/doc/blob/master/spec-rtmp_specification_1.0.pdf

c0 and s0
Version numbers, fixed to 0x03

c1 and s1
| 4-byte timestamp time | 4-byte all-0 binary string | 1528-byte random binary string |
The top 4-byte timestamp is typically in milliseconds.

When nrm is acting as a client, time in c1 uses the millisecond portion of the current unix timestamp. When nrm acts as a server, if the client is judged to be in simple mode, it does not use the timestamp in c1 after parsing it. When sending s1, it is returning the 1536 bytes of c1 as is.

With the 4-byte binary string of all zeros, the server can determine that the client is using simple mode.

c2 and s2
| 4-byte timestamp time | 4-byte time2 | 1528-byte random binary string |
As per the documentation:

c2's time should be set to the time field in s1. c2's time2 should be set to the point in time when s1 was received. The time of s2 should be set to the time field in c1. time2 of s2 should be set to the point in time when c1 was received.

When nrm is used as a server, if the client is judged to be in simple mode, the 1536 bytes of c1 are returned as is when s2 is sent.

If you use the obs client (obs uses simple handshake mode) to handshake with the nrm server, you will find that the entire 1536 bytes of c1, c2, s1, and s2 are identical. Showing that the fields time and time2, nrm doesn't do exactly what the documentation says.

Complex Mode
hmac-sha256
Before introducing complex mode, a hash signature algorithm is introduced, the hmac-sha256 algorithm. Complex mode will use it to do some signature operations and verification.

Briefly, the input of this algorithm is a key (length can be arbitrary) and an input string (length can be arbitrary), after hmac-sha256 operation to get a 32-byte signature string.

When the key and input are fixed, the hmac-sha256 result is also fixed and unique.

c0
Fixed to 0x03

c1
The format is as follows:

| 4-byte timestamp time | 4-byte mode string | 1528-byte complex binary string |
The 4-byte timestamp time field is described with reference to time in simple mode. ffmpeg uses [0, 0, 0, 0]

4-byte pattern string, nrm uses [0x0C, 0x00, 0x0D, 0x0E]. ffmpeg uses [9, 0, 124, 2]

The 1528-byte complex binary string generation rules are as follows:

Step one, randomise the 1528 byte complex binary string.

Step 2, write a 32-byte digest signature in the 1528-byte random binary string.

Location of digest
Firstly, we explain how the location of digest is determined. digest can be located in the first half or in the second half.

When digest is in the first half, the location information of digest (hereafter referred to as offset) is stored at the start of the first half.
The c1 format is expanded as follows:

| 4-byte time | 4-byte pattern string | 4-byte offset | left[...] | 32 bytes digest | right[...] | 764 bytes for the second half |
offset = (c1[8] + c1[9] + c1[10] + c1[11]) % 728 + 12
The calculated offset is relative to the starting position of the entire c1.

The reason for the balance of 728 is because the first half of the 764 bytes is subtracted from the 4 bytes of the offset field, and then subtracted from the 32 bytes of digest. The reason for adding 12 is to skip 4 bytes of time + 4 bytes of pattern string + 4 bytes of offset.

The value range of offset is [12,740).

When offset=12, the left part does not exist, and when offset=739, the right part does not exist.

When digest is in the second half, offset is saved at the start of the second half.
The c1 format is expanded as follows:

| 4-byte time | 4-byte pattern string | 764-byte first half | 4-byte offset | left[...] | 32 bytes digest | right[...] | 4 bytes offset = (c1[8+8])
offset = (c1[8+764] + c1[8+764+1] + c1[8+764+2] + c1[8+764+3]) % 728 + 8 + 764 + 4
The computed offset dependency is relative to the actual position of c1.

Why take the remainder 728? Because the second half of the 764 bytes is subtracted from the 4 bytes of the offset field, and then subtracted from the 32 bytes of digest. Why add 8 plus 764 plus 4? It's because you have to skip 4 bytes of time + 4 bytes of pattern string + 764 bytes of the first half + 4 bytes of offset.

The value range of offset is [776,1504).

When offset=776, the left part does not exist, and when offset=1503, the right part does not exist.

When nrm constructs c1 as a client, it uses the first format, where digest is placed in the first half.

How digest is generated
After talking about the location of digest, let's talk about how digest is generated.

That is, the left part of the c1 digest is spliced with the right part of the c1 digest (if the right part exists) as hmac-sha256 input (the size of the whole is 1536-32), the following size of 30 bytes of a fixed key as the hmac-sha256 key, through the hmac-sha256 calculations to derive a 32-byte digest is filled into the digest field in c1.

'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ', '
'f', 'l', 'a', 's', 'h', ' ', 'p', 'l', 'a', 'y', 'e', 'r', ' ','
'0', '0', '1'.
After receiving c1, the server side will first pass the mode string in c1 to initially determine whether it is a complex mode, and if it is a complex mode, it will re-digest through c1 to see whether the calculated digest is the same as the contained digest field in c1 to determine whether the handshake is a complex mode.

Note that since the server cannot directly know whether the client puts digest in the first half or the second half, the server can only verify one of them first, and if the verification fails, then verify the other, and if they both fail, consider backing off to use the simple mode to continue the handshake with the client.

s0
Fixed to 0x03

s1
The construction method of s1 is the same as c1. Only the pattern string is replaced with [0x0D, 0x0E, 0x0A, 0x0D]. And replace the hmac-sha256 key with the following 36-byte fixed key

'G', 'e', 'n', 'u', 'i', 'n', 'e', ' ', 'A', 'd', 'o', 'b', 'e', ' ', '
'f', 'l', 'a', 's', 'h', ' ', 'm', 'e', 'd', 'i', 'a', ' '.
's', 'e', 'r', 'v', 'e', 'r', ' ','
'0', '0', '1', '
s2
The format is as follows:

| 4-byte timestamp time | 4-byte time2 | 1528-byte random binary string |
where the time and time2 fields refer to the description of s2 in simple mode.

The 1528-byte random binary string also needs to be filled with digest.

The practice of nrm is to fill the 32-byte digest directly into the end of s2, i.e., the corresponding offset is not set. digest is computed by using the left part of digest as the hmac-sha256 input (size is 1536-32), using the digest in c1 as the hmac-sha256 key, and calculate digest by hmac-sha256.

c2
The construction method of c2 is the same as s2. Only it uses the digest in s2 as the hmac-sha256 key.

The original is not easy to reproduce, please indicate the article from the open source streaming server lal, Github: https://github.com/q191201771/lal Official Documentation: https://pengrl.com/lal

yoko, 201906
