package ru.nsu.dmustakaev.socksProxy;

import java.nio.ByteBuffer;

public abstract class Connection implements Handler {
    private boolean disconnect = false;

    abstract void linkBuffer(ByteBuffer clientBuffer);

    public void setDisconnect(boolean disconnect) {
        this.disconnect = disconnect;
    }

    public boolean isDisconnect() {
        return disconnect;
    }
}
