/**
 * 
 */
package com.taobao.middleware.competition.server;

import java.io.BufferedOutputStream;
import java.io.BufferedReader;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Arrays;

public class Server {
	private ServerSocket serverSocket;
	private String file;

	public Server(String file) {
		this.file = file;
	}

	public void init(int port) throws IOException {
		serverSocket = new ServerSocket();
		serverSocket.bind(new InetSocketAddress(port));
		while (true) {
			InputStream in = null;
			DataOutputStream out = null;
			BufferedReader fileReader = null;
			Socket s = null;
			try {
				s = serverSocket.accept();
				s.setSendBufferSize(128 * 1024);
				s.setReceiveBufferSize(128 * 1024);
				s.setTcpNoDelay(true);

				in = s.getInputStream();
				out = new DataOutputStream(new BufferedOutputStream(s.getOutputStream()));
				fileReader = new BufferedReader(new InputStreamReader(new FileInputStream(file)));
				while (true) {
					int inbyte = in.read();
					if (inbyte == 0) {
						processLine(fileReader, out);
					} else if (inbyte == -1) {
						break;
					}
				}
			} catch (Throwable e) {
				e.printStackTrace();
			} finally {
				if (s != null) {
					s.close();
				}
				if (in != null) {
					in.close();
				}
				if (out != null) {
					out.close();
				}
				if (fileReader != null) {
					fileReader.close();
				}
			}
		}
	}

	private void processLine(BufferedReader reader, DataOutputStream out) throws IOException {
		String line = reader.readLine();
		if (line != null) {
			byte[] b = line.getBytes();
			int idx = b.length / 3;
			int idx2 = idx + idx;
			out.writeInt(b.length - idx);
			reverse(b,0,idx,idx2,b.length - idx2);
			out.write(b, 0, idx);
			out.write(b, idx2, b.length - idx2);

		} else {
			out.writeInt(-1);
		}
		out.flush();
	}

	public static void reverse(byte[] buff, int s1, int s1Len, int s2, int s2Len) {
		int lp = s1;
		int rp = s2 + s2Len - 1;
		int lpe = s1 + s1Len;
		int rpe = s2 - 1;

		byte temp;
		while (true) {
			if (lp == lpe) {
				lp = s2;
			}
			if (rp == rpe) {
				rp = lpe - 1;
			}
			if (lp >= rp)
				break;
			temp = buff[lp];
			buff[lp] = buff[rp];
			buff[rp] = temp;
			++lp;
			--rp;
		}
	}

	public static void main(String[] args) throws IOException {
		if (args.length < 2) {
			throw new RuntimeException("need sourcefile port!");
		}
		Server server = new Server(args[0]);
		server.init(Integer.parseInt(args[1]));
	}
}
