package ru.nsu.dmustakaev.server.exception;

public class StartServerSocketException extends RuntimeException {
    public StartServerSocketException(String message) {
        super(message);
    }

    public StartServerSocketException(String message, Throwable cause) {
        super(message, cause);
    }
}
