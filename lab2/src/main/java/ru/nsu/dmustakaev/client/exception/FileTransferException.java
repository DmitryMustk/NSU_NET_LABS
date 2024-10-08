package ru.nsu.dmustakaev.client.exception;

public class FileTransferException extends RuntimeException {
    public FileTransferException(String message) {
        super(message);
    }

    public FileTransferException(String message, Throwable cause) {
        super(message, cause);
    }
}
