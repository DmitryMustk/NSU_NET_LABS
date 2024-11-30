package ru.nsu.dmustakaev.socksProxy;

public interface Handler {
    void handleEvent();
    void close();
}
