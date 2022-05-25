


Network Working Group                                 H. Alvestrand, Ed.
Internet-Draft                                                    Google
Intended status: Experimental                           October 21, 2013
Expires: April 24, 2014


          RTCP message for Receiver Estimated Maximum Bitrate
                     draft-alvestrand-rmcat-remb-03

Abstract

   This document proposes an RTCP message for use in experimentally-
   deployed congestion control algorithms for RTP-based media flows.

   It also describes an absolute-value timestamp option for use in
   bandwidth estimatoin.

Requirements Language

   The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL NOT",
   "SHOULD", "SHOULD NOT", "RECOMMENDED", "MAY", and "OPTIONAL" in this
   document are to be interpreted as described in RFC 2119 [RFC2119].

Status of this Memo

   This Internet-Draft is submitted in full conformance with the
   provisions of BCP 78 and BCP 79.

   Internet-Drafts are working documents of the Internet Engineering
   Task Force (IETF).  Note that other groups may also distribute
   working documents as Internet-Drafts.  The list of current Internet-
   Drafts is at http://datatracker.ietf.org/drafts/current/.

   Internet-Drafts are draft documents valid for a maximum of six months
   and may be updated, replaced, or obsoleted by other documents at any
   time.  It is inappropriate to use Internet-Drafts as reference
   material or to cite them other than as "work in progress."

   This Internet-Draft will expire on April 24, 2014.

Copyright Notice

   Copyright (c) 2013 IETF Trust and the persons identified as the
   document authors.  All rights reserved.

   This document is subject to BCP 78 and the IETF Trust's Legal
   Provisions Relating to IETF Documents
   (http://trustee.ietf.org/license-info) in effect on the date of



Alvestrand               Expires April 24, 2014                 [Page 1]

Internet-Draft                    REMB                      October 2013


   publication of this document.  Please review these documents
   carefully, as they describe your rights and restrictions with respect
   to this document.  Code Components extracted from this document must
   include Simplified BSD License text as described in Section 4.e of
   the Trust Legal Provisions and are provided without warranty as
   described in the Simplified BSD License.


Table of Contents

   1.  Introduction  . . . . . . . . . . . . . . . . . . . . . . . . . 3
   2.  Receiver Estimated Max Bitrate (REMB) . . . . . . . . . . . . . 3
     2.1.  Semantics . . . . . . . . . . . . . . . . . . . . . . . . . 3
     2.2.  Message format  . . . . . . . . . . . . . . . . . . . . . . 3
     2.3.  Signaling of use of this extension  . . . . . . . . . . . . 5
   3.  Absolute Send Time  . . . . . . . . . . . . . . . . . . . . . . 5
   4.  IANA considerations . . . . . . . . . . . . . . . . . . . . . . 6
   5.  Security Considerations . . . . . . . . . . . . . . . . . . . . 6
   6.  Acknowledgements  . . . . . . . . . . . . . . . . . . . . . . . 6
   7.  References  . . . . . . . . . . . . . . . . . . . . . . . . . . 7
     7.1.  Normative References  . . . . . . . . . . . . . . . . . . . 7
     7.2.  Informative References  . . . . . . . . . . . . . . . . . . 7
   Appendix A.  Change log . . . . . . . . . . . . . . . . . . . . . . 7
     A.1.  From appendix of -congestion-01 to -00  . . . . . . . . . . 7
     A.2.  From -00 to -02 . . . . . . . . . . . . . . . . . . . . . . 7
     A.3.  From -02 to -03 . . . . . . . . . . . . . . . . . . . . . . 8
   Author's Address  . . . . . . . . . . . . . . . . . . . . . . . . . 8
























Alvestrand               Expires April 24, 2014                 [Page 2]

Internet-Draft                    REMB                      October 2013


1.  Introduction

   This document proposes an RTCP feedback message signalling the
   estimated total available bandwidth for a session.

   If this function is available, it is possible to implement the
   algorithm in [I-D.alvestrand-rtcweb-congestion], or other algorithms
   with the same kind of feedback messaging need, in a fashion that
   covers multiple RTP streams at once.


2.  Receiver Estimated Max Bitrate (REMB)

2.1.  Semantics

   This feedback message is used to notify a sender of multiple media
   streams over the same RTP session of the total estimated available
   bit rate on the path to the receiving side of this RTP session.

   Within the common packet header for feedback messages (as defined in
   section 6.1 of [RFC4585]), the "SSRC of packet sender" field
   indicates the source of the notification.  The "SSRC of media source"
   is not used and SHALL be set to 0.  This usage of the value zero is
   also done in other RFCs.

   The reception of a REMB message by a media sender conforming to this
   specification SHALL result in the total bit rate sent on the RTP
   session this message applies to being equal to or lower than the bit
   rate in this message.  The new bit rate constraint should be applied
   as fast as reasonable.  The sender is free to apply additional
   bandwidth restrictions based on its own restrictions and estimates.

2.2.  Message format

   This document describes a message using the application specific
   payload type.  This is suitable for experimentation; upon
   standardization, a specific type can be assigned for the purpose.

   The message is an RTCP message with payload type 206.  RFC 3550
   [RFC3550] defines the range, RFC 4585 defines the specific PT value
   206 and the FMT value 15.










Alvestrand               Expires April 24, 2014                 [Page 3]

Internet-Draft                    REMB                      October 2013


    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |V=2|P| FMT=15  |   PT=206      |             length            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  SSRC of packet sender                        |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                  SSRC of media source                         |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Unique identifier 'R' 'E' 'M' 'B'                            |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  Num SSRC     | BR Exp    |  BR Mantissa                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |   SSRC feedback                                               |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |  ...                                                          |


   The fields V, P, SSRC, and length are defined in the RTP
   specification [2], the respective meaning being summarized below:

   version (V): (2 bits):   This field identifies the RTP version.  The
               current version is 2.

   padding (P) (1 bit):   If set, the padding bit indicates that the
               packet contains additional padding octets at the end that
               are not part of the control information but are included
               in the length field.  Always 0.

   Feedback message type (FMT) (5 bits):  This field identifies the type
               of the FB message and is interpreted relative to the type
               (transport layer, payload- specific, or application layer
               feedback).  Always 15, application layer feedback
               message.  RFC 4585 section 6.4.

   Payload type (PT) (8 bits):   This is the RTCP packet type that
               identifies the packet as being an RTCP FB message.
               Always PSFB (206), Payload-specific FB message.  RFC 4585
               section 6.4.

   Length (16 bits):  The length of this packet in 32-bit words minus
               one, including the header and any padding.  This is in
               line with the definition of the length field used in RTCP
               sender and receiver reports [3].  RFC 4585 section 6.4.







Alvestrand               Expires April 24, 2014                 [Page 4]

Internet-Draft                    REMB                      October 2013


   SSRC of packet sender (32 bits):  The synchronization source
               identifier for the originator of this packet.  RFC 4585
               section 6.4.

   SSRC of media source (32 bits):  Always 0; this is the same
               convention as in [RFC5104] section 4.2.2.2 (TMMBN).

   Unique identifier (32 bits):  Always 'R' 'E' 'M' 'B' (4 ASCII
               characters).

   Num SSRC (8 bits):  Number of SSRCs in this message.

   BR Exp (6 bits):   The exponential scaling of the mantissa for the
               maximum total media bit rate value, ignoring all packet
               overhead.  The value is an unsigned integer [0..63], as
               in RFC 5104 section 4.2.2.1.

   BR Mantissa (18 bits):   The mantissa of the maximum total media bit
               rate (ignoring all packet overhead) that the sender of
               the REMB estimates.  The BR is the estimate of the
               traveled path for the SSRCs reported in this message.
               The value is an unsigned integer in number of bits per
               second.

   SSRC feedback (32 bits)  Consists of one or more SSRC entries which
               this feedback message applies to.

2.3.  Signaling of use of this extension

   We negotiate use of the message in SDP using a header extension
   according to RFC 4585 section 4.2, with the value "goog-remb":

   a=rtcp-fb:<payload type> goog-remb


3.  Absolute Send Time

   Google has found that there are issues with relative send time offset
   when packets are relayed at nodes that are not the source of the RTP
   clock; it is hard to generate accurate offsets when you have to
   regenerate the base clock from the incoming packets before you can
   figure out what time to match; also, using signals from multiple
   flows becomes impossible unless the timestamps come from a common
   clock.

   The Absolute Send Time extension is used to stamp RTP packets with a
   timestamp showing the departure time from the system that put this
   packet on the wire (or as close to this as we can manage).



Alvestrand               Expires April 24, 2014                 [Page 5]

Internet-Draft                    REMB                      October 2013


   o  Name: "Absolute Sender Time" ; "RTP Header Extension for Absolute
      Sender Time".

   o  Formal name:
      "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time".

   o  Wire format: 1-byte extension, 3 bytes of data. total 4 bytes
      extra per packet (plus shared 4 bytes for all extensions present:
      2 byte magic word 0xBEDE, 2 byte # of extensions).

   o  Encoding: Timestamp is in seconds, 24 bit 6.18 fixed point,
      yielding 64s wraparound and 3.8us resolution (one increment for
      each 477 bytes going out on a 1Gbps interface).

   o  Relation to NTP timestamps: abs_send_time_24 = (ntp_timestamp_64
      >> 14) & 0x00ffffff ; NTP timestamp is the number of seconds since
      the epoch, in 32.32 bit fixed point format.

   o  Notes: Packets are time stamped when going out, preferably close
      to metal.  Intermediate RTP relays (RTP entities possibly altering
      the relative timing of packets in the stream) should remove the
      extension or overwrite its value with its own timestamp.

   When signalled in SDP, the standard mechanism for RTCP extensions
   [RFC5285] is used:

   a=extmap:3 http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time


4.  IANA considerations

   Upon publication of this document as an RFC (if it is decided to
   publish it), IANA is requested to register the string "goog-remb" in
   its registry of "rtcp-fb" values in the SDP attribute registry group.


5.  Security Considerations

   If the RTCP packet is not protected, it is possible to inject fake
   RTCP packets that can increase or decrease bandwidth.  This is not
   different from security considerations for any other RTCP message.


6.  Acknowledgements

   This proposal has emerged from discussions between, among others,
   Justin Uberti, Magnus Flodman, Patrik Westin, Stefan Holmer and
   Henrik Lundin.



Alvestrand               Expires April 24, 2014                 [Page 6]

Internet-Draft                    REMB                      October 2013


7.  References

7.1.  Normative References

   [RFC2119]  Bradner, S., "Key words for use in RFCs to Indicate
              Requirement Levels", BCP 14, RFC 2119, March 1997.

   [RFC3550]  Schulzrinne, H., Casner, S., Frederick, R., and V.
              Jacobson, "RTP: A Transport Protocol for Real-Time
              Applications", STD 64, RFC 3550, July 2003.

   [RFC4585]  Ott, J., Wenger, S., Sato, N., Burmeister, C., and J. Rey,
              "Extended RTP Profile for Real-time Transport Control
              Protocol (RTCP)-Based Feedback (RTP/AVPF)", RFC 4585,
              July 2006.

   [RFC5104]  Wenger, S., Chandra, U., Westerlund, M., and B. Burman,
              "Codec Control Messages in the RTP Audio-Visual Profile
              with Feedback (AVPF)", RFC 5104, February 2008.

   [RFC5285]  Singer, D. and H. Desineni, "A General Mechanism for RTP
              Header Extensions", RFC 5285, July 2008.

7.2.  Informative References

   [I-D.alvestrand-rtcweb-congestion]
              Holmer, S. and H. Alvestrand, "A Google Congestion Control
              Algorithm for Real-Time Communication on the World Wide
              Web", draft-alvestrand-rtcweb-congestion-03 (work in
              progress), October 2012.

   [RFC5450]  Singer, D. and H. Desineni, "Transmission Time Offsets in
              RTP Streams", RFC 5450, March 2009.


Appendix A.  Change log

A.1.  From appendix of -congestion-01 to -00

   The timestamp option was removed.  Discussion concluded that the RFC
   5450 [RFC5450] "transmission time offset" header likely gives
   accurate enough send-time information for our purposes.

A.2.  From -00 to -02

   No changes.  These are "keepalive" publications.





Alvestrand               Expires April 24, 2014                 [Page 7]

Internet-Draft                    REMB                      October 2013


A.3.  From -02 to -03

   Added information on the absolute-timestamp extension and on SDP
   negotiation of REMB.


Author's Address

   Harald Alvestrand (editor)
   Google
   Kungsbron 2
   Stockholm,   11122
   Sweden

   Email: harald@alvestrand.no




































Alvestrand               Expires April 24, 2014                 [Page 8]

