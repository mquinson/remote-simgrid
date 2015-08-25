import org.simgrid.rsg.Actor;
import org.simgrid.rsg.Mailbox;

public class dumbClient {
	public static void main(String[] args) {
		Mailbox mbox = Mailbox.byName("toto");
		Actor self = Actor.self();
		self.sleep(42);
		self.execute(8095000000.); // That's the power of my host on the used platform

		self.send(mbox,"message from client");
		String msg = self.recv(mbox);
		System.err.println("jClient: Received message: "+msg);
		
		self.quit();
	}
}