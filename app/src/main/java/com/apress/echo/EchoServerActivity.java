package com.apress.echo;

/**
 * Echo server.
 * <p>
 * author : suikajy
 * date : 2018/9/4
 */
public class EchoServerActivity extends AbstractEchoActivity {
    /**
     * Constructor.
     */
    public EchoServerActivity(int layoutID) {
        super(layoutID);
    }

    @Override
    protected void onStartButtonClicked() {
        Integer port = getPort();
        if (port != null) {
            ServerTask serverTask = new ServerTask(port);
            serverTask.start();
        }
    }

    /**
     * Starts the TCP server on the given port.
     *
     * @param port port number.
     */
    private native void nativeStartTcpServer(int port) throws Exception;

    /**
     * Starts the UDP server on the given port.
     *
     * @param port port number.
     */
    private native void nativeStartUdpServer(int port) throws Exception;

    /**
     * Server task.
     */
    private class ServerTask extends AbstractEchoTask {
        /**
         * Port number.
         */
        private final int port;

        /**
         * Constructor.
         *
         * @param port port number.
         */
        public ServerTask(int port) {
            this.port = port;
        }

        protected void onBackground() {
            logMessage("Starting server.");
            try {
                nativeStartTcpServer(port);
            } catch (Exception e) {
                logMessage(e.getMessage());
            }
            logMessage("Server terminated.");
        }
    }
}
