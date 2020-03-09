import java.io.*;
import java.net.*;
import java.util.*;
import java.util.concurrent.*;

/** Router module for an overlay router.
 *
 *  Your documentation here
 *  Should describe the protocol used by the router and a high level
 *  description of when various packets are sent and how received
 *  packets are processed
 */
public class Router implements Runnable {
	private Thread myThread;		// thread that executes run() method
	private int myIp;			// ip address in the overlay
	private String myIpString;		// String representation
	private ArrayList<Prefix> pfxList;	// list of prefixes to advertise
	private ArrayList<NborInfo> nborList;	// list of info about neighbors
	private class LinkInfo { 		// class used to record link information
		public int peerIp;		// IP address of peer in overlay net
		public double cost;		// in seconds
		public boolean gotReply;	// flag to detect hello replies
		public int helloState;		// set to 3 when hello reply received
						// decremented whenever hello reply
						// is not received; when 0, link is down

		// link cost statistics
		public int count;
		public double totalCost;
		public double minCost;
		public double maxCost;

		LinkInfo() {
			cost = 0; gotReply = true; helloState = 3;
			count = 0; totalCost = 0; minCost = 10; maxCost = 0;
		}
	}
	private ArrayList<LinkInfo> lnkVec;     // indexed by link number

	private class Route { 		        // routing table entry
		public Prefix pfx;	        // destination prefix for route
		public double timestamp;        // time this route was generated
		public double cost;	        // cost of route in ns
		public LinkedList<Integer> path;// list of router IPs;
						// destination at end of list
		public int outLink;		// outgoing link for this route
		public boolean valid;		//indicate the valid of the route
	}
	private ArrayList<Route> rteTbl;        // routing table

	private Forwarder fwdr;		        // reference to Forwarder object

	private double now;								// current time in ns
	private static final double sec = 1000000000; 	// ns per sec

	private int debug;		// controls debugging output
	private boolean quit;		// stop thread when true
	private boolean enFA;		// link failure advertisement enable


	/** Initialize a new Router object.
	 *  
	 *  @param myIp is an integer representing the overlay IP address of
	 *  this node in the overlay network
	 *  @param fwdr is a reference to the Forwarder object through which
	 *  the Router sends and receives packets
	 *  @param pfxList is a list of prefixes advertised by this router
	 *  @param nborList is a list of neighbors of this node
	 *
	 *  @param debug is an integer that controls the amount of debugging
	 *  information that is to be printed
	 */

	Router(int myIp, Forwarder fwdr, ArrayList<Prefix> pfxList,
			ArrayList<NborInfo> nborList, int debug, boolean enFA) {
		this.myIp = myIp; this.myIpString = Util.ip2string(myIp);
		this.fwdr = fwdr; this.pfxList = pfxList;
		this.nborList = nborList; this.debug = debug;
		this.enFA = enFA;

		lnkVec = new ArrayList<LinkInfo>();
		for (NborInfo nbor : nborList) {
			LinkInfo lnk = new LinkInfo();
			lnk.peerIp = nbor.ip;
			lnk.cost = nbor.delay;
			lnkVec.add(lnk);
		}
		rteTbl = new ArrayList<Route>();
		quit = false;
	}

	/** Instantiate and start a thread to execute run(). */
	public void start() {
		myThread = new Thread(this); myThread.start();
	}

	/** Terminate the thread. */
	public void stop() throws Exception { quit = true; myThread.join(); }

	/** This is the main thread for the Router object.
	 *
	 * Your documentation here
	 */
	public void run() {
		double t0 = System.nanoTime()/sec;
		now = 0;
		double helloTime, pvSendTime;
		helloTime = pvSendTime = now;
		while (!quit) {
			// TODO
			// if it's time to send hello packets, do it
			// else if it's time to send advertisements, do it
			// else if the forwarder has an incoming packet
			// to be processed, retrieve it and process it
			// else nothing to do, so take a nap
			now = System.nanoTime()/sec - t0;
			if (helloTime == 0 || (now - helloTime) > = 1)
			{
				sendHellos();
				helloTime = now;
			}
			else if (pvSendTime == 0 || (now - pvSendTime) >= 10)
			{
				sendAdverts();
				pvSendTime = now;
			}
			else if (fwdr.incoming())
			{
				handleIncoming();
			}
			else {
				try { Thread.sleep(1); } catch(Exception e) {
					System.out.println("Router: "
							+ "can't sleep");
					System.exit(1);
				}
			}
		}
		String s = String.format("Router link cost statistics\n" + 
			"%8s %8s %8s %8s %8s\n","peerIp","count","avgCost",
			"minCost","maxCost");
		for (LinkInfo lnk : lnkVec) {
			if (lnk.count == 0) continue;
			s += String.format("%8s %8d %8.3f %8.3f %8.3f\n",
				Util.ip2string(lnk.peerIp), lnk.count,
				lnk.totalCost/lnk.count,
				lnk.minCost, lnk.maxCost);
		}
		System.out.println(s);
	}

	/** Lookup route in routing table.
	 *
	 * @param pfx is IP address prefix to be looked up.
	 * @return a reference to the Route that matches the prefix or null
	 */
	private Route lookupRoute(Prefix pfx) {
		for (Route rte : rteTbl)
		{
			if (rte.pfx.equals(pfx)) return rte;
		}
		return null;
	}

	/** Add a route to the routing table.
	 * 
	 *  @param rte is a route to be added to the table; no check is
	 *  done to make sure this route does not conflict with an existing
	 *  route
	 */
	private void addRoute(Route rte) {
		rteTbl.add(rte);
	}

	 /** Update a route in the routing table.
	 *
	 *  @param rte is a reference to a route in the routing table.
	 *  @param nuRte is a reference to a new route that has the same
	 *  prefix as rte
	 *  @return true if rte is modified, else false
	 *
	 *  This method replaces certain fields in rte with fields
	 *  in nuRte. Specifically,
	 *
	 *  if nuRte has a link field that refers to a disabled
	 *  link, ignore it and return false
	 *
	 *  else, if the route is invalid, then update the route
	 *  and return true,
	 *
	 *  else, if both routes have the same path and link,
	 *  then the timestamp and cost fields of rte are updated
	 *
	 *  else, if nuRte has a cost that is less than .9 times the
	 *  cost of rte, then all fields in rte except the prefix fields
	 *  are replaced with the corresponding fields in nuRte
	 *
	 *  else, if nuRte is at least 20 seconds newer than rte
	 *  (as indicated by their timestamps), then all fields of
	 *  rte except the prefix fields are replaced
	 *
	 *  else, if the link field for rte refers to a link that is
	 *  currently disabled, replace all fields in rte but the
	 *  prefix fields
	 */
	private boolean updateRoute(Route rte, Route nuRte) {
		// TODO update route
		// outlink disabled
		if (lnkVec.get(nuRte.outLink).helloState == 0) return false;
		if (!rte.valid)
		{
			rte.timestamp = nuRte.timestamp;
			rte.cost = nuRte.cost;
			rte.outLink = nuRte.outLink;
			rte.valid = nuRte.valid;
			return true;
		}
		boolean samePathSameLnk = false;
		if (rte.path.size() == nuRte.path.size() && 
			rte.outLink == nuRte.outLink) 
		{
			for (int i = 0; i < rte.path.size(); ++i)
			{
				samePathSameLnk = (rte.path.get(i) != nuRte.path.get(i)) ?
					false : true;

			}
		}
		if (samePathSameLnk)
		{
			rte.timestamp = nuRte.timestamp;
			rte.cost = nuRte.cost;
			return true;
		}
		if (nuRte.cost < 0.9*rte.cost ||
			 (nuRte.timestamp - rte.timestamp) >= 20)
		{
			rte.timestamp = nuRte.timestamp;
			rte.cost = nuRte.cost;
			rte.outLink = nuRte.outLink;
			rte.valid = nuRte.valid;
			return true;
		}
		if (lnkVec.get(rte.outLink).helloState == 0)
		{
			rte.timestamp = nuRte.timestamp;
			rte.cost = nuRte.cost;
			rte.outLink = nuRte.outLink;
			rte.valid = nuRte.valid;
			return true;
		}
	}
				
	/** Send hello packet to all neighbors.
	 *
	 *  First check for replies. If no reply received on some link,
	 *  update the link status by subtracting 1. If that makes it 0,
	 *  the link is considered down, so we mark all routes using 
	 *  that link as invalid. Also, if certain routes are marked as 
	 *  invalid, we will need to print the table if debug larger 
	 *  than 1, and we need to send failure advertisement by 
	 *  calling sendFailureAdvert if failure advertisement is enable.
	 */
	public void sendHellos() {
		int lnk = 0;
		for (LinkInfo lnkInfo : lnkVec) {
			//TODO
			// if no reply to the last hello, subtract 1 from
			// link status if it's not already 0
			
			// go through the routes to check routes 
			// that contain the failed link
			
			// print routing table if debug is enabled 
			// and valid field of route is changed
			
			// send link failure advertisement if enFA is enabled
			// and valid field of route is changed

			// send new hello, after setting gotReply to false
			if (!lnkInfo.gotReply && lnkInfo.helloState != 0)
			{
				lnkInfo.helloState -= 1;
				//3 hello miss
				if (lnkInfo.helloState == 0)
				{
					for (Route rte : rteTbl)
					{
						if (rte.outLink == lnk)
						{
							rte.valid == false;
							if (enFA) sendFailureAdvert(lnk);
							if (debug > 0) printTable();
						}
					}
				}
			}
			lnkInfo.gotReply = false;
			Packet p = new Packet();
			p.protocol = 2; p.ttl = 100;
			p.srcAdr = myIp;
			p.destAdr = lnkInfo.peerIp;
			p.payload = String.format("RPv0\ntype: hello\n"
				+ "timestamp: %.3f\n", now);
			if (fwdr.ready4pkt()) fwdr.send(p, lnk);
			lnk += 1;
		}
	}

	/** Send initial path vector to each of our neighbors.  */
	public void sendAdverts() {
		// TODO send advertisement
		for (int lnk = 0; lnk < nborList.size(); lnk++) {
			if (lnkVec.get(lnk).helloState == 0) continue;
			Packet p = new Packet();
			p.protocol = 2; p.ttl = 100;
			p.srcAdr = myIp;
			p.destAdr = lnkVec.get(lnk).peerIp;
			for (Prefix pfx : pfxList)
			{
				p.payload = String.format("RPv0\ntype: advert\n")
				if (rteTbl.isEmpty() || lookupRoute(pfx) == null){
					p.payload = String.format(p.payload +
						"pathvec: %s %.3f %.3f %s\n", 
						pfx.toString(), now, 0, myIpString);
					if (fwdr.ready4pkt()) fwdr.sendPkt(p,lnk);
				}
			}
		}
	}


	/** Send link failure advertisement to all available neighbors
	 *
	 *  @param failedLnk is the number of link on which is failed.
	 *
	 */
	public void sendFailureAdvert(int failedLnk){
		int failIp = lnkVec.get(failedLnk).peerIp;
		String failIpString = Util.ip2string(failIp);

		for (int lnk = 0; lnk < nborList.size(); lnk++) {
			if (lnkVec.get(lnk).helloState == 0) continue;
			Packet p = new Packet();
			p.protocol = 2; p.ttl = 100;
			p.srcAdr = myIp;
			p.destAdr = lnkVec.get(lnk).peerIp;
			p.payload = String.format("RPv0\ntype: fadvert\n"
				+ "linkfail: %s %s %.3f %s\n",
				myIpString,  failIpString, now, myIpString);
			fwdr.sendPkt(p,lnk);
		}
	}

	/** Retrieve and process packet received from Forwarder.
	 *
	 *  For hello packets, we simply echo them back.
	 *  For replies to our own hello packets, we update costs.
	 *  For advertisements, we update routing state and propagate
	 *  as appropriate.
	 */
	public void handleIncoming() {
		// parse the packet payload
		Pair<Packet,Integer> pp = fwdr.receivePkt();
		Packet p = pp.left; int lnk = pp.right;

		String[] lines = p.payload.split("\n");
		if (!lines[0].equals("RPv0")) return;

		String[] chunks = lines[1].split(":");
		if (!chunks[0].equals("type")) return;
		String type = chunks[1].trim();

		// TODO
		// if it's an route advert, call handleAdvert
		// if it's a link failure advert, call handleFailureAdvert
		// if it's a hello, echo it back
		// else it's a reply to a hello packet
		// use timestamp to determine round-trip delay
		// use this to update the link cost using exponential
		// weighted moving average method
		// also, update link cost statistics
		// also, set gotReply to true
		if (chunks[1].equals("advert")) 
		{
			String[] advert = lines[2].split(":");
			if (!advert[0].equals("pathvec")) return;
			handleAdvert(lines, lnk);
		}
		if (chunks[1].equals("fadvert")) 
		{
			String[] fadvert = lines[2].split(":");
			if (!advert[0].equals("linkfail")) return;
			handleFailureAdvert(lines, lnk);
		}
		if (chunks[1].equals("hello"))
		{
			String[] hello = lines[2].split(":");
			if (!advert[0].equals("timestamp")) return;
			p.payload = String.format("RPv0\ntype: hello2u\n" + lines[2]);
			if (fwdr.ready4pkt()) fwdr.sendPkt(p,lnk);
		}
		if (chunks[1].equals("hello2u"))
		{
			String[] hello = lines[2].split(":");
			if (!advert[0].equals("timestamp")) return;
			double d = Double.parseDouble(advert[1].trim());
			d = (now - d)/2;
			lnkVec.get(lnk).count++;
			lnkVec.get(lnk).cost = 0.1*d + 0.9*lnkVec.get(lnk).cost;
			lnkVec.get(lnk).totalCost += lnkVec.get(lnk).cost;
			lnkVec.get(lnk).minCost = Math.min(lnkVec.get(lnk).minCost,
				 lnkVec.get(lnk).cost);
			lnkVec.get(lnk).maxCost = Math.max(lnkVec.get(lnk).maxCost,
				 lnkVec.get(lnk).cost);
			lnkVec.get(lnk).gotReply = true;
		}
	}

	/** Handle an advertisement received from another router.
	 *
	 *  @param lines is a list of lines that defines the packet;
	 *  the first two lines have already been processed at this point
	 *
	 *  @param lnk is the number of link on which the packet was received
	 */
	private void handleAdvert(String[] lines, int lnk) {
		// example path vector line
		// pathvec: 1.2.0.0/16 345.678 .052 1.2.0.1 1.2.3.4
	        //
		// TODO
        // Parse the path vector line.
        // If there is loop in path vector, ignore this packet.
        // Form a new route, with cost equal to path vector cost
        // plus the cost of the link on which it arrived.
        // Look for a matching route in the routing table
        // and update as appropriate; whenever an update
        // changes the path, print the table if debug>0;
        // whenever an update changes the output link,
        // update the forwarding table as well.
        // If the new route changed the routing table,
        // extend the path vector and send it to other neighbors.
		if (!lnkVec.get(lnk).valid) return;
		Route rte = new Route();
		String[] advert = lines[2].split(":");
		String[] rteStat = advert[1].trim().split(" ");
		Prefix pfx = new Prefix(rteStat[0].trim());
		double timestamp = Double.parseDouble(rteStat[1].trim());
		double cost = Double.parseDouble(rteStat[2].trim());
		for (int i = 3; i < rteStat.length(); ++i)
		{
			if (rteStat[i].trim().equals(myIpString)) return;
			rte.path.add(Util.string2ip(rteStat[i].trim()));
		}
		rte.cost = lnkVec.get(lnk).cost + cost;
		rte.pfx = pfx;
		rte.timestamp = timestamp;
		rte.valid = true;
		rte.outLink = lnk;

		Route oldRoute = lookupRoute(pfx);
		if (oldRoute == null || updateRoute(oldRoute, rte))
		{
			if (oldRoute == null) addRoute(rte);
			if (debug > 0) printTable();
			// forward advertisement to other peers
			lines[2] = advert[0] + ": " rteStat[0] + " " +
				rteStat[1] + " " + Double.toString(rte.cost) + " " +
				myIpString;
			for (int i = 3; i < rteStat.length(); ++i)
			{
				lines[2] = lines[2] + " " + rteStat[i];
			}
			Packet p = new Packet();
			for (int i = 0; i < nborList.size(); i++)
			{
				if (i == lnk) continue;
				p.protocol = 2;
				p.srcAdr = myIp;
				p.destAdr = lnkVec.get(i).peerIp;
				p.ttl = 100;
				p.payload = lines.toString();
				if (fwdr.ready4pkt()) fwdr.sendPkt(p, i);
			}
			// update forwarding table
			rte = lookupRoute(pfx);
			fwdr.addRoute(rte.pfx, rte.outLink);
		}

	}

	/** Handle the failure advertisement received from another router.
	 *
	 *  @param lines is a list of lines that defines the packet;
	 *  the first two lines have already been processed at this point
	 *
	 *  @param lnk is the number of link on which the packet was received
	 */
	private void handleFailureAdvert(String[] lines, int lnk) {
		// example path vector line
		// fadvert: 1.2.0.1 1.3.0.1 345.678 1.4.0.1 1.2.0.1
		// meaning link 1.2.0.1 to 1.3.0.1 is failed
	        //
		// TODO
        // Parse the path vector line.
        // If there is loop in path vector, ignore this packet.
        
        // go through routes to check if it contains the link
		// set the route as invalid (false) if it does
		
		// update the time stamp if route is changed
		// print route table if route is changed and debug is enabled
		
		// If one route is changed, extend the message 
		// and send it to other neighbors.
		if (!lnkVec.get(lnk).valid) return;
		String[] fadvert = lines[2].split(":");
		String[] link = fadvert[1].split(" ");
		//Route rte = new Route();
		for (int i = 3; i < link.length(); ++i)
		{
			if (link[i].trim().equals(myIpString)) return;
			//rte.path.add(Util.string2ip(link[i].trim()));
		}
		boolean changed = false;
		for (Route rte : rteTbl)
		{
			for (int i = 0; i < rte.path.size(); ++i)
			{
				if (rte.path.get(i) == Util.string2ip(link[0]) &&
					i+1 < rte.path.size() &&
					rte.path.get(i+1) == Util.string2ip(link[1]))
				{
					rte.valid = false; 
					rte.timestamp = Double.parseDouble(link[2]);
					changed = true;
					if (debug > 0) printTable();
				}
			}
		}
		lines[2] = fadvert[0] + ": " + lines[0] + " " + lines[1] + " " +
			lines[2] + " " + myIpString;
		for (int i = 3; i < rteStat.length(); ++i)
		{
			lines[2] = lines[2] + " " + link[i];
		}
		Packet p = new Packet();
		for (int i = 0; i < nborList.size(); i++)
		{
			if (i == lnk) continue;
			p.protocol = 2;
			p.srcAdr = myIp;
			p.destAdr = lnkVec.get(i).peerIp;
			p.ttl = 100;
			p.payload = lines.toString();
			if (fwdr.ready4pkt()) fwdr.sendPkt(p, i);
		}

	}

	/** Print the contents of the routing table. */
	public void printTable() {
		String s = String.format("Routing table (%.3f)\n"
			+ "%10s %10s %8s %5s %10s \t path\n", now, "prefix", 
			"timestamp", "cost","link", "VLD/INVLD");
		for (Route rte : rteTbl) {
			s += String.format("%10s %10.3f %8.3f",
				rte.pfx.toString(), rte.timestamp, rte.cost);
			
			s += String.format(" %5d", rte.outLink);
			
			if (rte.valid == true)
				s+= String.format(" %10s", "valid");
			else
				s+= String.format(" %10s \t", "invalid");
			
			for (int r :rte.path)
				s += String.format (" %s",Util.ip2string(r));
			
			if (lnkVec.get(rte.outLink).helloState == 0)
				s += String.format("\t ** disabled link");
			s += "\n";
		}
		System.out.println(s);
	}
}
