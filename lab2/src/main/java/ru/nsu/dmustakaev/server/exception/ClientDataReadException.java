package ru.nsu.dmustakaev.server.exception;

public class ClientDataReadException extends RuntimeException {
    public ClientDataReadException(String message) {
        super(message);
    }

    public ClientDataReadException(String message, Throwable cause) {
        super(message, cause);
    }
}
