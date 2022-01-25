package frc.robot;

import java.util.ArrayList;

import edu.wpi.first.networktables.NetworkTableInstance;
import edu.wpi.first.networktables.NetworkTableEntry;
import edu.wpi.first.networktables.NetworkTable;

// comment out this import and all commandbased-related code at the bottom of the file if you are not using the command library
import edu.wpi.first.wpilibj2.command.CommandBase;


public class VisionServer {

	public final NetworkTable 
		root, targets, cameras, pipelines;
	private final NetworkTableEntry 
		active_target, num_cams, cam_idx, num_pipes, pipe_idx;
	private ArrayList<VsCamera> vscameras = new ArrayList<VsCamera>();
	private ArrayList<VsPipeline> vspipelines = new ArrayList<VsPipeline>();

	// singleton
	private VisionServer() {
		root = NetworkTableInstance.getDefault().getTable("Vision Server");
		targets = NetworkTableInstance.getDefault().getTable("Targets");
		cameras = root.getSubTable("Cameras");
		pipelines = root.getSubTable("Pipelines");

		active_target = root.getEntry("Active Target");
		num_cams = root.getEntry("Cameras Available");
		cam_idx = root.getEntry("Camera Index");
		num_pipes = root.getEntry("Pipelines Available");
		pipe_idx = root.getEntry("Pipeline Index");

		this.updateCameras();
		this.updatePipelines();
	}
	public static VisionServer Get() { return VisionServer.global; }
	private static final VisionServer global = new VisionServer();

	public void updateCameras() {
		this.vscameras.clear();
		for(String subtable : cameras.getSubTables()) {
			this.vscameras.add(new VsCamera(subtable));
		}
	}
	public void updatePipelines() {
		this.vspipelines.clear();
		for(String subtable : pipelines.getSubTables()) {
			this.vspipelines.add(new VsPipeline(subtable));
		}
	}
	public ArrayList<VsCamera> getCameras() { return this.vscameras; }
	public ArrayList<VsPipeline> getPipelines() { return this.vspipelines; }
	public VsCamera getCamera(int idx) { return idx < this.vscameras.size() ? idx > 0 ? this.vscameras.get(idx) : null : null; }
	public VsCamera getCamera(String name) {
		for(int i = 0; i < this.vscameras.size(); i++) {
			if(this.vscameras.get(i).name.equals(name)) {
				return this.vscameras.get(i);
			}
		}
		return null;
	}
	public VsPipeline getPipeline(int idx) { return idx < this.vspipelines.size() ? idx > 0 ? this.vspipelines.get(idx) : null : null; }
	public VsPipeline getPipeline(String name) {
		for(int i = 0; i < this.vspipelines.size(); i++) {
			if(this.vspipelines.get(i).name.equals(name)) {
				return this.vspipelines.get(i);
			}
		}
		return null;
	}
	public VsCamera getCurrentCamera() { return this.getCamera(this.getCameraIdx()); }
	public VsPipeline getCurrentPipeline() { return this.getPipeline(this.getPipelineIdx()); }

	public boolean getIsShowingStatistics() { return root.getEntry("Show Statistics").getBoolean(false); }
	public void setStatistics(boolean val) { root.getEntry("Show Statistics").setBoolean(val); }
	public void toggleStatistics() { this.setStatistics(!this.getIsShowingStatistics()); }

	public boolean getIsPipelineEnabled() { 
		if(root.containsKey("Enable Processing")) {
			return root.getEntry("Enable Processing").getBoolean(true);
		}
		return false;
	}
	public boolean setPipelineEnabled(boolean val) {
		if(root.containsKey("Enable Processing")) {
			return root.getEntry("Enable Processing").setBoolean(val);
		}
		return false;
	}
	public boolean togglePipelineEnabled() {
		if(root.containsKey("Enable Processing")) {
			return root.getEntry("Enable Processing").setBoolean(!root.getEntry("Enable Processing").getBoolean(true));
		}
		return false;
	}

	public boolean hasActiveTarget() { return !active_target.getString("none").equals("none"); }
	public NetworkTable getActiveTarget() { return targets.getSubTable(active_target.getString("none")); }
	public String getActiveTargetName() { return active_target.getString("none"); }
	public double getDistance() { return this.getActiveTarget().getEntry("distance").getDouble(0.0); }
	public double getThetaUD() { return this.getActiveTarget().getEntry("up-down").getDouble(0.0); }
	public double getThetaLR() { return this.getActiveTarget().getEntry("left-right").getDouble(0.0); }
	public TargetOffset getTargetPos() { return new TargetOffset(this.getActiveTarget()); }
	public TargetData getTargetData() { return new TargetData(this.getActiveTarget()); }

	public int numCameras() { return (int)num_cams.getDouble(0.0); }	// returns 0 on failure
	public int getCameraIdx() { return (int)cam_idx.getDouble(-1.0); }	// returns -1 on failure
	public boolean setCamera(int idx) {		// returns whether the input index was valid or not
		cam_idx.setDouble(idx);
		return idx < this.numCameras() && idx > 0;
	}
	public boolean incrementCamera() {
		int idx = this.getCameraIdx();
		if(idx + 1 < this.numCameras()) {
			cam_idx.setDouble(idx + 1);
			return true;
		}
		cam_idx.setDouble(0.0);	// wrap around
		return false;
	}
	public boolean decrementCamera() {
		int idx = this.getCameraIdx();
		if(idx - 1 >= 0) {
			cam_idx.setDouble(idx - 1);
			return true;
		}
		cam_idx.setDouble(this.numCameras() - 1);	// wrap around
		return false;
	}
	public int numPipelines() { return (int)num_pipes.getDouble(0.0); }		// returns 0 on failure
	public int getPipelineIdx() { return (int)pipe_idx.getDouble(-1.0); }	// returns -1 on failure
	public boolean setPipeline(int idx) {	// returns whether the input index was valid or not
		pipe_idx.setDouble(idx);
		return idx < this.numPipelines() && idx > 0;
	}
	public boolean incrementPipeline() {
		int idx = this.getPipelineIdx();
		if(idx + 1 < this.numPipelines()) {
			pipe_idx.setDouble(idx + 1);
			return true;
		}
		pipe_idx.setDouble(0.0);	// wrap around
		return false;
	}
	public boolean decrementPipeline() {
		int idx = this.getPipelineIdx();
		if(idx - 1 >= 0) {
			pipe_idx.setDouble(idx - 1);
			return true;
		}
		pipe_idx.setDouble(this.numPipelines() - 1);	// wrap around
		return false;
	}



	public static class VsCamera {

		private NetworkTable self;
		private String name;

		public void update(NetworkTable nt) {
			this.self = nt;
			this.name = NetworkTable.basenameKey(nt.getPath());
		}
		public void update(String tname) {
			this.self = VisionServer.Get().cameras.getSubTable(name);
			this.name = tname;
		}
		public VsCamera(NetworkTable nt) { this.update(nt); }
		public VsCamera(String name) { this.update(name); }

		public String getName() { return this.name; }
		public NetworkTable get() { return this.self; }

		public int getExposure() { return (int)this.self.getEntry("Exposure").getDouble(0.0); }
		public int getBrightness() { return (int)this.self.getEntry("Brightness").getDouble(0.0); }
		public int getWhiteBalance() {return (int)this.self.getEntry("WhiteBalance").getDouble(0.0); }
		public boolean setExposure(int e) { return this.self.getEntry("Exposure").setDouble(e); }
		public boolean setBrightness(int b) { return this.self.getEntry("Brightness").setDouble(b); }
		public boolean setWhiteBalance(int wb) { return this.self.getEntry("WhiteBalance").setDouble(wb); }
		public NetworkTableEntry getExposureEntry() { return this.self.getEntry("Exposure"); }
		public NetworkTableEntry getBrightnessEntry() { return this.self.getEntry("Brightness"); }
		public NetworkTableEntry getWhiteBalanceEntry() { return this.self.getEntry("WhiteBalance"); }

		public String toString() {
			return this.getClass().getName() + '(' + this.name + ")@" + Integer.toHexString(this.hashCode()) + 
				": {EX: " + this.getExposure() + ", BR: " + this.getBrightness() + ", WB: " + this.getWhiteBalance() + '}';
		}

	}
	public static class VsPipeline {

		private NetworkTable self;
		private String name;
		NetworkTableEntry debug = null, thresh = null;

		public void update(NetworkTable nt) {
			this.self = nt;
			this.name = NetworkTable.basenameKey(nt.getPath());
		}
		public void update(String tname) {
			this.self = VisionServer.Get().pipelines.getSubTable(tname);
			this.name = tname;
		}
		public VsPipeline(NetworkTable nt) {
			this.update(nt);
		}
		public VsPipeline(String tname) {
			this.update(tname);
		}

		public String getName() { return this.name; }
		public NetworkTable get() { return this.self; }
		
		public NetworkTableEntry[] getEntries() {
			NetworkTableEntry entries[] = new NetworkTableEntry[this.self.getKeys().size()];
			int i = 0;
			for(String entry : this.self.getKeys()) {
				entries[i] = (this.self.getEntry(entry));
				i++;
			}
			return entries;
		}
		public NetworkTableEntry searchEntries(String segment) {
			String lower = segment.toLowerCase();
			for(String key : this.self.getKeys()) {
				if(key.toLowerCase().contains(lower)) {
					return this.self.getEntry(key);
				}
			}
			return null;
		}
		public NetworkTableEntry[] searchEntries(String[] segments) {
			NetworkTableEntry ret[] = new NetworkTableEntry[segments.length];
			String kbuff;
			for(String key : this.self.getKeys()) {
				kbuff = key.toLowerCase();
				for(int i = 0; i < segments.length; i++) {
					if(kbuff.contains(segments[i].toLowerCase())) {
						ret[i] = this.self.getEntry(key);
					}
				}
			}
			return ret;
		}
		public void searchUsableEntries() {
			String buff;
			for(String key : this.self.getKeys()) {
				buff = key.toLowerCase();
				if(buff.contains("debug")) {
					this.debug = this.self.getEntry(key);
				}
				if(buff.contains("threshold")) {	// else if?
					this.thresh = this.self.getEntry(key);
				}
			}
		}
		public boolean hasDebug() {
			if(this.debug == null) {
				this.searchUsableEntries();
				if(this.debug == null) {
					return false;
				}
			}
			return true;
		}
		public boolean hasThreshold() {
			if(this.thresh == null) {
				this.searchUsableEntries();
				if(this.debug == null) {
					return false;
				}
			}
			return true;
		}
		public boolean setDebug(boolean val) {
			if(this.debug == null) {
				this.searchUsableEntries();
				if(this.debug == null) {
					return false;
				}
			}
			return this.debug.setBoolean(val);
		}
		public boolean setThreshold(boolean val) {
			if(this.thresh == null) {
				this.searchUsableEntries();
				if(this.debug == null) {
					return false;
				}
			}
			return this.thresh.setBoolean(val);
		}

	}

	public static class TargetOffset {
		public double x, y, z;
		// rotation also when that gets implemented in the networktables
		public TargetOffset(double x, double y, double z) {
			this.x = x;
			this.y = y;
			this.z = z;
		}
		public TargetOffset(NetworkTable target) {
			this.x = target.getEntry("x").getDouble(0.0);
			this.y = target.getEntry("y").getDouble(0.0);
			this.z = target.getEntry("z").getDouble(0.0);
		}
	}
	public static class TargetData {
		public TargetOffset pos;
		public double distance, ud, lr;

		public TargetData(double x, double y, double z, double d, double ud, double lr) {
			this.pos = new TargetOffset(x, y, z);
			this.distance = d;
			this.ud = ud;
			this.lr = lr;
		}
		public TargetData(TargetOffset pos, double d, double ud, double lr) {
			this.pos = pos;
			this.distance = d;
			this.ud = ud;
			this.lr = lr;
		}
		public TargetData(NetworkTable target) {
			this.pos = new TargetOffset(target);
			this.distance = target.getEntry("distance").getDouble(0.0);
			this.ud = target.getEntry("up-down").getDouble(0.0);
			this.lr = target.getEntry("left-right").getDouble(0.0);
		}
	}



	// Command-based -> comment out if not using the commands library 

	private final IncrementCamera inc_camera = new IncrementCamera();
	private final DecrementCamera dec_camera = new DecrementCamera();
	private final IncrementPipeline inc_pipeline = new IncrementPipeline();
	private final DecrementPipeline dec_pipeline = new DecrementPipeline();
	private final ToggleStatistics toggle_stats = new ToggleStatistics();
	private final TogglePipeline toggle_pipeline = new TogglePipeline();

	public static IncrementCamera getCameraIncrementCommand() { return global.inc_camera; }
	public static DecrementCamera getCameraDecrementCommand() { return global.dec_camera; }
	public static IncrementPipeline getPipelineIncrementCommand() { return global.inc_pipeline; }
	public static DecrementPipeline getPipelineDecrementCommand() { return global.dec_pipeline; }
	public static ToggleStatistics getToggleStatisticsCommand() { return global.toggle_stats; }
	public static TogglePipeline getPipelineToggleCommand() { return global.toggle_pipeline; }

	private class IncrementCamera extends CommandBase {
		public IncrementCamera() {}
		@Override public void initialize() { incrementCamera(); }
		@Override public boolean isFinished() { return true; }
	}
	private class DecrementCamera extends CommandBase {
		public DecrementCamera() {}
		@Override public void initialize() { decrementCamera(); }
		@Override public boolean isFinished() { return true; }
	}
	private class IncrementPipeline extends CommandBase {
		public IncrementPipeline() {}
		@Override public void initialize() { incrementPipeline(); }
		@Override public boolean isFinished() { return true; }
	}
	private class DecrementPipeline extends CommandBase {
		public DecrementPipeline() {}
		@Override public void initialize() { decrementPipeline(); }
		@Override public boolean isFinished() { return true; }
	}
	private class ToggleStatistics extends CommandBase {
		public ToggleStatistics() {}
		@Override public void initialize() { toggleStatistics(); }
		@Override public boolean isFinished() { return true; }
	}
	private class TogglePipeline extends CommandBase {
		public TogglePipeline() {}
		@Override public void initialize() { togglePipelineEnabled(); }
		@Override public boolean isFinished() { return true; }
	}
// 	private static class ToggleDebug extends CommandBase {		// not implemented yet
}