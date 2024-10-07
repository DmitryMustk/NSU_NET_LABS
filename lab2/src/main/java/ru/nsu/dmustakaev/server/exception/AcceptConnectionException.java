package ru.nsu.dmustakaev.server.exception;

public class AcceptConnectionException extends RuntimeException {
    public AcceptConnectionException(String message) {
        super(message);
    }

    public AcceptConnectionException(String message, Throwable cause) {
        super(message, cause);
    }
}
