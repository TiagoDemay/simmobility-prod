package sim_mob.vis.network;

import java.awt.*;
import java.awt.geom.Rectangle2D;

import sim_mob.vis.MainFrame;
import sim_mob.vis.controls.DrawParams;
import sim_mob.vis.controls.DrawableItem;
import sim_mob.vis.network.basic.ScaledPoint;
import sim_mob.vis.util.Utility;

/**
 * \author Zhang Shuai
 * \author Seth N. Hetu
 * \author Matthew Bremer Bruchon
 */
public class LaneMarking implements DrawableItem{
	//Constants/Resources
	//private static Color laneColor = new Color(0x00, 0x00, 0x00);
	//private static Color sideWalkColor = new Color(0x84, 0x70, 0xff);
	//private static Stroke laneStroke = new BasicStroke(1.0F);
	
	private Long parentSegment;
	private ScaledPoint start;
	private ScaledPoint end;
	private ScaledPoint startPt;
	private ScaledPoint secondPt;
	private ScaledPoint penultimatePt; //second to last
	private ScaledPoint lastPt;
	
	private boolean isSideWalk;
	private int laneNumber;
	
	
	public int getZOrder() {
		return DrawableItem.Z_ORDER_LANEMARKING;
	}
	

	public LaneMarking(ScaledPoint start, ScaledPoint end, boolean isSideWalk, int lineNumber, Long parentSegment) {		
		this.start = start;
		this.end = end;
		this.isSideWalk = isSideWalk;
		this.laneNumber = lineNumber;
		this.parentSegment = parentSegment;

		if(start != null)
		{
			this.startPt = start;
			this.secondPt = start;			
		}
		if(end != null)
		{
			this.penultimatePt = end;
			this.lastPt = end;
		}
	}
	
	
	public Rectangle2D getBounds() {
		final double BUFFER_CM = 10*100; //1m
		Rectangle2D res = new Rectangle2D.Double(start.getUnscaledX(), start.getUnscaledY(), 0, 0);
		res.add(end.getUnscaledX(), end.getUnscaledY());
		Utility.resizeRectangle(res, res.getWidth()+BUFFER_CM, res.getHeight()+BUFFER_CM);
		return res;
	}
	

	public ScaledPoint getStart() { return start; }
	public ScaledPoint getEnd() { return end; }
	public boolean isSideWalk() { return isSideWalk; }
	public int getLaneNumber()	{ return laneNumber; }
	public Long getParentSegment(){ return parentSegment; }

	public void setStartPt(ScaledPoint pt){ startPt = pt; }
	public void setSecondPt(ScaledPoint pt){ secondPt = pt; }
	public void setPenultimatePt(ScaledPoint pt){ penultimatePt = pt; }
	public void setLastPt(ScaledPoint pt){ lastPt = pt; }

	public void setSideWalk(boolean isSideWalk){
		this.isSideWalk = isSideWalk;
	}
	
	@Override
	public void draw(Graphics2D g, DrawParams params) {
		if (!params.PastCriticalZoom) { return; }
		
		//Retrieve
		Color clr = MainFrame.Config.getLineColor(isSideWalk?"sidewalk":"lane");
		Stroke strk = MainFrame.Config.getLineStroke(isSideWalk?"sidewalk":"lane");
		
		//Override for lane line zero
		if (laneNumber==0) {
			clr = MainFrame.Config.getLineColor("median");
			strk = MainFrame.Config.getLineStroke("median");
		}
		
		//Draw it.
		g.setColor(clr);
		g.setStroke(strk);
				
		//NOTE: All of these are somewhat wrong; we need to see exactly how startPt/lastPt etc. are being set.
		//   startPt, secondPt seem wrong.
		//   penultimatePt, lastPt are correct
		//Compare their access patterns!
		g.drawLine((int)startPt.getX(),(int)startPt.getY(),(int)secondPt.getX(),(int)secondPt.getY());
		g.drawLine((int)secondPt.getX(),(int)secondPt.getY(),(int)penultimatePt.getX(),(int)penultimatePt.getY());
		g.drawLine((int)penultimatePt.getX(),(int)penultimatePt.getY(), (int)lastPt.getX(),(int)lastPt.getY());
	}

	
}
