package exewrap.util;

import java.io.IOException;
import java.io.OutputStream;
import java.io.PrintStream;

import exewrap.core.ExewrapClassLoader;

public class ConsoleOutputStream extends OutputStream {
	static {
		System.setOut(new PrintStream(new ConsoleOutputStream()));
	}

	@Override
	public void write(int b) throws IOException {
		ExewrapClassLoader.WriteConsole(new byte[] { (byte)(b & 0xFF) }, 0, 1);
	}

	@Override
	public void write(byte[] b) throws IOException {
		if(b != null) {
			ExewrapClassLoader.WriteConsole(b, 0, b.length);
		}
	}

	@Override
	public void write(byte[] b, int off, int len) throws IOException {
		if(b != null) {
			ExewrapClassLoader.WriteConsole(b, off, len);
		}
	}
}
