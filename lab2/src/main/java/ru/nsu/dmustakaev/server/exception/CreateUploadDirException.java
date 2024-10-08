package ru.nsu.dmustakaev.server.exception;

public class CreateUploadDirException extends RuntimeException {
    public CreateUploadDirException(String message) {
        super(message);
    }

    public CreateUploadDirException(String message, Throwable cause) {
        super(message, cause);
    }
}
