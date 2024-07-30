## protocol ideas

### outline

- layer 1: PHY
    - can be uart, rs232, rs485, rs422 - anything that allows sending a stream of bytes
    - more than two devices on the same bus are allowed, but only one is the master
- layer 2: HDLC-like
    - stock HDLC has the following frame fields:
        - optional start byte 0x7E (sent only if there has been no activity on the link for some time)
        - address of the slave (1 byte)
        - flow control byte
            - P/F bit: a token that indicates the start/end of a request/response sequence; only useful if slaves can send more than one frame at a time
            - sequence number, 3 bits - only useful if we allow multiple frames in flight
            - other flow control data, varies by frame type, mostly useful for acknowledge/retransmission of in-flight frames
        - data
        - checksum (2 bytes)
        - end byte (0x7E)
- layer 3: messages
    - message ID (either one byte, or we can do something fancy like UTF-8 and make it two bytes if the MSB is set)
    - struct that contains the message

### ideas
items marked with 🤔 should be discussed with priority

- 🤔 one-message-per-frame, one frame in flight:
    - if we design the library with only this use case in mind, it can be simplified a great deal
    - no need for message length fields or termination between messages, the whole frame is always a message
    - no need for P/F bit, sequence numbers, etc
- message types
    - request-response
        - the master sends a message to a specific slave and waits for a response
        - the slave responds with another message
        - 🤔 maybe have a few reserved message IDs for "request specific message without arguments", "ok", "error response", etc
    - pubsub
        - the master may send a reserved "subscribe" message that makes the slave send the message periodically
        - 🤔 this may be stupid for the following reasons:
            - very difficult to implement correctly in multi-slave topologies
            - will require special logic for simplex links like rs485, which will incur overhead that is the same as requesting the message every time
        - 🤔 instead of doing this, we can either:
            - drop it entirely and have the master poll for telemetry messages individually
            - have some way to request several messages at once (but this will be tricky to implement - when do we know if the message stream has finished? what if the "finished" marker is corrupted? there is no way to implement this without blocking for a certain timeout)
- 🤔 if all communication is request-response (which is basically the only thing that works on rs485), we can probably drop the flow control field entirely, and assume that the slave will respond to every message from the master. Upon receiving a corrupted frame, the slave can send an error response.

### message format

- messages are structs that may contain:
    - integers (signed/unsigned, 8/16/32/64 bits)
    - floats (32/64 bits, maybe 16?)
    - chars
    - enums
    - flag fields
    - arrays
    - structs
    - it would be great to have unions (enums could also work like this)
- messages may be extended by adding more fields at the end
- ideally, we would like to have a flat structure that can be directly sent as a message in-place without reencoding
- we want a language-agnostic way to define messages, and a way to compile these definitions to C, Go, etc
- 🤔 in particular, the generated C code should ideally be just structs/types with some extra macros that deal with things like getting/setting flags in flag fields
    - alternatively, the generated C code could also be a bunch of getter/setter macros that operate on a raw data buffer - this is also optimal for our case
- what we do **NOT** want is:
    - generated code that calls `malloc()` anywhere
    - lots of overhead (e.g. field IDs, pointers, padding)

#### evaluation of different binary definition libraries I found:
- 🤮 ProtoBuf, Apache Thrift, etc: way too much overhead, no C support, rely on field IDs for everything
- 🤮 MsgPack, CBOR, or anything similar that strives to be schema-less: field IDs that are strings, unusable
- 👎 CapnProto - might work, but has hardcoded 8-byte padding for some values and the status of the C library is unclear
- 👎 FlatBuffers - has two different data types for structs: *table* and *struct*. Tables have field IDs, pointers and generally take lots of space (up to 8x the space for data). In theory, the `flatcc` tool supports "root structs", which means that we might be able to make the library work for static types without tables. This is unsupported in the official library, though, but there might be a hacky way to get it to work. In any case, we will definitely not be using this library as intended.
- 👎 SBE - no available implementation for C, and the implementation for other languages is too heavy. The definition language itself may work fine.
- 🤔 ASN.1 - the PER encoding of ASN.1 might work, but the popular C libraries I could find rely on allocating stuff too much. The definition language has everything nice.
    - 🤔 Actually, the library [asn1scc](https://github.com/maxime-esa/asn1scc) seems to have exactly the features we need, is developed by ESA and they actually use it in space

Out of these libraries, the only one that seems optimised for resource-constrained environments, at least from what I could find, is asn1scc.
The other alternative, of course, is writing our own.
