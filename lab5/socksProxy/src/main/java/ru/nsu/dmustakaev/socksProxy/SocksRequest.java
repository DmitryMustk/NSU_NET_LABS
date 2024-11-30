package ru.nsu.dmustakaev.socksProxy;

import java.nio.ByteBuffer;

public class SocksRequest {
    private static final int NO_AUTH_METHOD           = 0x00;
    private static final int ADDRESS_TYPE_IPV4        = 0x01;
    private static final int ADDRESS_TYPE_DOMAIN_NAME = 0x03;
    private static final int ADDRESS_TYPE_IPV6        = 0x04;

    private final ByteBuffer buffer;
    private final OperationType type;

    public SocksRequest(ByteBuffer buffer, OperationType type) {
        this.buffer = buffer;
        this.type = type;
    }

    public boolean isRequestLengthValid() {
        return switch (type) {
            case HELLO -> isHelloRequestLengthValid();
            case HEADER -> isHeaderRequestLengthValid();
            case MESSAGE -> true;
        };
    }

    // requestLength > 1 && requestLength >= (VerByte + NMethByte) + NMeth
    private boolean isHelloRequestLengthValid() {
        return buffer.position() > 1 && buffer.position() >= 2 + buffer.get(1);
    }

    /*
    +----+-----+-------+------+----------+----------+
    |VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
    +----+-----+-------+------+----------+----------+
    | 1  |  1  | X'00' |  1   | Variable |    2     |
    +----+-----+-------+------+----------+----------+
     */
    private boolean isHeaderRequestLengthValid() {
        if (buffer.position() < 5) {
            return false;
        }

        return switch (buffer.get(3)) {
            case ADDRESS_TYPE_IPV4 -> buffer.position() == 10;
            case ADDRESS_TYPE_DOMAIN_NAME -> buffer.position() == 7 + buffer.get(4);
            case ADDRESS_TYPE_IPV6 -> buffer.position() == 22;

            default -> true;
        };
    }

    public boolean isRequestValid() {
        return switch (type) {
            case HELLO -> isHelloRequestValid();
            case HEADER -> isHeaderRequestValid();
            case MESSAGE -> true;
        };
    }

    private boolean isHelloRequestValid() {
        return checkMethod();
    }

    private boolean isHeaderRequestValid() {
        if (buffer.get(1) != 0x01) {
            return false;
        }
        return buffer.get(3) != 0x04;
    }

    /*
       +----+----------+----------+
       |VER | NMETHODS | METHODS  |
       +----+----------+----------+
       | 1  |    1     | 1 to 255 |
       +----+----------+----------+

       o  X'00' NO AUTHENTICATION REQUIRED
       o  X'01' GSSAPI
       o  X'02' USERNAME/PASSWORD
       o  X'03' to X'7F' IANA ASSIGNED
       o  X'80' to X'FE' RESERVED FOR PRIVATE METHODS
       o  X'FF' NO ACCEPTABLE METHODS
     */
    private boolean checkMethod() {
        for (int i = 0; i < buffer.get(1); ++i) {
            if (buffer.get(i + 2) == NO_AUTH_METHOD) {
                return true;
            }
        }
        return false;
    }
}
