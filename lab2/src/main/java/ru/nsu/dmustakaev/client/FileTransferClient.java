package ru.nsu.dmustakaev.client;

import ru.nsu.dmustakaev.client.exception.FileTransferException;

import java.io.*;
import java.net.Socket;

public class FileTransferClient {
    private static final int BUF_SIZE = 8192;

    private final String host;
    private final int port;

    public FileTransferClient(String host, int port, String filepath) {
        this.host = host;
        this.port = port;
    }

    public void transferFile(String filepath) {
        try (Socket socket = new Socket(host, port);
             DataOutputStream out = new DataOutputStream(socket.getOutputStream());
             DataInputStream in = new DataInputStream(socket.getInputStream())
        ){
            File file = new File(filepath);

            transferHeader(file, out);
            transferBytes(file, out);
            checkServerResponse(in);
        } catch (IOException e) {
            throw new FileTransferException("Can't transfer file", e);
        }
    }

    private void transferHeader(File file, DataOutputStream out) throws IOException {
        long fileSize = file.length();
        String fileName = file.getName();

        out.writeUTF(fileName);
        out.writeLong(fileSize);
    }

    private void transferBytes(File file, DataOutputStream out) throws IOException {
        try (FileInputStream fis = new FileInputStream(file)) {
            byte[] buffer = new byte[BUF_SIZE];
            int bytesRead;
            while ((bytesRead = fis.read(buffer)) != -1) {
                out.write(buffer, 0, bytesRead);
            }
        } catch (FileNotFoundException e) {
            throw new FileTransferException("File not found", e);   
        }
    }

    private void checkServerResponse(DataInputStream in) throws IOException {
        String serverResponse = in.readUTF();
        if (serverResponse.equals("SUCCESS")) {
            System.out.println("File transfer successful");
        } else {
            System.out.println("File transfer failed");
        }
    }
}
