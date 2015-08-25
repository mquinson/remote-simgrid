package org.simgrid.rsg;

public class Actor {
	private Actor() {
		
	}
	private static Actor instance = null;
	public static Actor self() {
		if (instance == null) 
			instance = new Actor();
		return instance;
	}
	public void sleep(double duration) {
		Rsg.Request req = Rsg.Request.newBuilder()
				.setType(Rsg.Type.CMD_SLEEP)
				.setSleep(Rsg.Request.SleepRequest.newBuilder().setDuration(duration))
				.build();
		Engine.getInstance().sendRequest(req);
	}
	public void execute(int flops) {
		execute((double)flops);
	}
	public void execute(double flops) {
		Rsg.Request req = Rsg.Request.newBuilder()
				.setType(Rsg.Type.CMD_EXEC)
				.setExec(Rsg.Request.ExecRequest.newBuilder().setFlops(flops))
				.build();
		Engine.getInstance().sendRequest(req);
	}
	public void quit() {
		Rsg.Request req = Rsg.Request.newBuilder()
				.setType(Rsg.Type.CMD_QUIT)
				.build();
		Engine.getInstance().sendRequest(req);
	}
	public void send(Mailbox mailbox, String content) {
		Rsg.Request req = Rsg.Request.newBuilder()
				.setType(Rsg.Type.CMD_SEND)
				.setSend(Rsg.Request.SendRequest.newBuilder()
						.setMbox(mailbox.getRemoteAddr())
						.setContent(content))
				.build();
		Engine.getInstance().sendRequest(req);
	}
	public String recv(Mailbox mailbox) {
		Rsg.Request req = Rsg.Request.newBuilder()
				.setType(Rsg.Type.CMD_RECV)
				.setRecv(Rsg.Request.RecvRequest.newBuilder()
						.setMbox(mailbox.getRemoteAddr()))
				.build();
		Rsg.Answer ans = Engine.getInstance().sendRequest(req);
		
		return ans.getRecv().getContent();
	}
}
