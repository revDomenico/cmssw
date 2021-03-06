///////////////////////////////////////////////////////////////////////////////
// File: DDHGCalEEAlgo.cc
// Description: Geometry factory class for HGCal (EE and HESil)
///////////////////////////////////////////////////////////////////////////////

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DetectorDescription/Core/interface/DDutils.h"
#include "DetectorDescription/Core/interface/DDSolid.h"
#include "DetectorDescription/Core/interface/DDMaterial.h"
#include "DetectorDescription/Core/interface/DDCurrentNamespace.h"
#include "DetectorDescription/Core/interface/DDSplit.h"
#include "Geometry/HGCalCommonData/plugins/DDHGCalEEAlgo.h"
#include "CLHEP/Units/GlobalPhysicalConstants.h"
#include "CLHEP/Units/GlobalSystemOfUnits.h"

//#define EDM_ML_DEBUG

DDHGCalEEAlgo::DDHGCalEEAlgo() {
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: Creating an instance";
#endif
}

DDHGCalEEAlgo::~DDHGCalEEAlgo() {}

void DDHGCalEEAlgo::initialize(const DDNumericArguments & nArgs,
			       const DDVectorArguments & vArgs,
			       const DDMapArguments & ,
			       const DDStringArguments & sArgs,
			       const DDStringVectorArguments &vsArgs) {

  wafers_       = vsArgs["WaferNames"];
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << wafers_.size() 
				<< " wafers";
  for (unsigned int i=0; i<wafers_.size(); ++i)
    edm::LogVerbatim("HGCalGeom") << "Wafer[" << i << "] " << wafers_[i];
#endif
  materials_    = vsArgs["MaterialNames"];
  names_        = vsArgs["VolumeNames"];
  thick_        = vArgs["Thickness"];
  for (unsigned int i=0; i<materials_.size(); ++i) {
    copyNumber_.emplace_back(1);
  }
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << materials_.size()
				<< " types of volumes";
  for (unsigned int i=0; i<names_.size(); ++i)
    edm::LogVerbatim("HGCalGeom") << "Volume [" << i << "] " << names_[i] 
				  << " of thickness " << thick_[i] 
				  << " filled with " << materials_[i]
				  << " first copy number " << copyNumber_[i];
#endif
  layers_       = dbl_to_int(vArgs["Layers"]);
  layerThick_   = vArgs["LayerThick"];
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "There are " << layers_.size() << " blocks";
  for (unsigned int i=0; i<layers_.size(); ++i)
    edm::LogVerbatim("HGCalGeom") << "Block [" << i << "] of thickness "  
				  << layerThick_[i] 
				  << " with " << layers_[i] << " layers";
#endif
  layerType_    = dbl_to_int(vArgs["LayerType"]);
  layerSense_   = dbl_to_int(vArgs["LayerSense"]);
  firstLayer_   = (int)(nArgs["FirstLayer"]);
  if (firstLayer_ > 0) {
    for (unsigned int i=0; i<layerType_.size(); ++i) {
      if (layerSense_[i] != 0) {
	int ii = layerType_[i];
	copyNumber_[ii] = firstLayer_;
#ifdef EDM_ML_DEBUG
	edm::LogVerbatim("HGCalGeom") << "First copy number for layer type "
				      << i << ":" << ii << " with "
				      << materials_[ii] << " changed to "
				      << copyNumber_[ii];
#endif
	break;
      }
    }
  }
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "There are " << layerType_.size() 
				<< " layers" ;
  for (unsigned int i=0; i<layerType_.size(); ++i)
    edm::LogVerbatim("HGCalGeom") << "Layer [" << i << "] with material type " 
				  << layerType_[i] << " sensitive class " 
				  << layerSense_[i];
#endif
  zMinBlock_    = nArgs["zMinBlock"];
  rMaxFine_     = nArgs["rMaxFine"];
  rMinThick_    = nArgs["rMinThick"];
  waferSize_    = nArgs["waferSize"];
  waferSepar_   = nArgs["SensorSeparation"];
  sectors_      = (int)(nArgs["Sectors"]);
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "zStart " << zMinBlock_ << " rFineCoarse " 
				<< rMaxFine_ << " rMaxThick " << rMinThick_
				<< " wafer width " << waferSize_ 
				<< " separations " << waferSepar_
				<< " sectors " << sectors_;
#endif
  slopeB_       = vArgs["SlopeBottom"];
  slopeT_       = vArgs["SlopeTop"];
  zFront_       = vArgs["ZFront"];
  rMaxFront_    = vArgs["RMaxFront"];
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "Bottom slopes " << slopeB_[0] << ":" 
				<< slopeB_[1] << " and " << slopeT_.size() 
				<< " slopes for top" ;
  for (unsigned int i=0; i<slopeT_.size(); ++i)
    edm::LogVerbatim("HGCalGeom") << "Block [" << i << "] Zmin " << zFront_[i]
				  << " Rmax " << rMaxFront_[i] << " Slope " 
				  << slopeT_[i];
#endif
  nameSpace_    = DDCurrentNamespace::ns();
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: NameSpace " << nameSpace_;
#endif
}

////////////////////////////////////////////////////////////////////
// DDHGCalEEAlgo methods...
////////////////////////////////////////////////////////////////////

void DDHGCalEEAlgo::execute(DDCompactView& cpv) {
  
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "==>> Constructing DDHGCalEEAlgo...";
  copies_.clear();
#endif
  constructLayers (parent(), cpv);
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << copies_.size() 
				<< " different wafer copy numbers";
  int k(0);
  for (std::unordered_set<int>::const_iterator itr=copies_.begin();
       itr != copies_.end(); ++itr,++k) {
    edm::LogVerbatim("HGCalGeom") << "Copy [" << k << "] : " << (*itr);
  }
  copies_.clear();
  edm::LogVerbatim("HGCalGeom") << "<<== End of DDHGCalEEAlgo construction...";
#endif
}

void DDHGCalEEAlgo::constructLayers(const DDLogicalPart& module, 
				    DDCompactView& cpv) {
  
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: \t\tInside Layers";
#endif
  double       zi(zMinBlock_);
  int          laymin(0);
  const double tol(0.01);
  for (unsigned int i=0; i<layers_.size(); i++) {
    double  zo     = zi + layerThick_[i];
    double  routF  = rMax(zi);
    int     laymax = laymin+layers_[i];
    double  zz     = zi;
    double  thickTot(0);
    std::vector<double> pgonZ(2), pgonRin(2), pgonRout(2);
    for (int ly=laymin; ly<laymax; ++ly) {
      int     ii     = layerType_[ly];
      int     copy   = copyNumber_[ii];
      double  hthick = 0.5*thick_[ii];
      double  rinB   = (layerSense_[ly] == 0) ? (zo*slopeB_[0]) :
	(zo*slopeB_[1]);
      zz            += hthick;
      thickTot      += thick_[ii];

      std::string name = "HGCal"+names_[ii]+std::to_string(copy);
#ifdef EDM_ML_DEBUG
      edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: Layer " << ly << ":" 
				    << ii << " Front " << zi << ", " << routF
				    << " Back " << zo << ", " << rinB 
				    << " superlayer thickness "
				    << layerThick_[i];
#endif
      DDName matName(DDSplit(materials_[ii]).first, 
		     DDSplit(materials_[ii]).second);
      DDMaterial matter(matName);
      DDLogicalPart glog;
      if (layerSense_[ly] == 0) {
	double alpha = CLHEP::pi/sectors_;
	double rmax  = routF*cos(alpha) - tol;
	pgonZ[0]    =-hthick;  pgonZ[1]    = hthick;
	pgonRin[0]  = rinB;    pgonRin[1]  = rinB;   
	pgonRout[0] = rmax;    pgonRout[1] = rmax;   
	DDSolid solid = DDSolidFactory::polyhedra(DDName(name, nameSpace_),
						  sectors_,-alpha,CLHEP::twopi,
						  pgonZ, pgonRin, pgonRout);
	glog = DDLogicalPart(solid.ddname(), matter, solid);
#ifdef EDM_ML_DEBUG
	edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << solid.name() 
				      << " polyhedra of " << sectors_ 
				      << " sectors covering " 
				      << -alpha/CLHEP::deg << ":" 
				      << (-alpha+CLHEP::twopi)/CLHEP::deg
				      << " with " << pgonZ.size()
				      << " sections and filled with "
				      << matName << ":" << &matter;
	for (unsigned int k=0; k<pgonZ.size(); ++k)
	  edm::LogVerbatim("HGCalGeom") << "[" << k << "] z " << pgonZ[k] 
					<< " R " << pgonRin[k] << ":" 
					<< pgonRout[k];
#endif
      } else {
	DDSolid solid = DDSolidFactory::tubs(DDName(name, nameSpace_), 
					     hthick, rinB, routF, 0.0,
					     CLHEP::twopi);
	glog = DDLogicalPart(solid.ddname(), matter, solid);
#ifdef EDM_ML_DEBUG
	edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << solid.name() 
				      << " Tubs made of " << matName << ":"
				      << &matter << " of dimensions " << rinB 
				      << ", " << routF << ", " << hthick
				      << ", 0.0, " << CLHEP::twopi/CLHEP::deg
				      << " and position " << glog.name() 
				      << " number " << copy;
#endif
	positionSensitive(glog,rinB,routF,layerSense_[ly],cpv);
      }
      DDTranslation r1(0,0,zz);
      DDRotation rot;
      cpv.position(glog, module, copy, r1, rot);
      ++copyNumber_[ii];
#ifdef EDM_ML_DEBUG
      edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << glog.name() 
				    << " number " << copy << " positioned in "
				    << module.name() << " at " << r1 
				    << " with " << rot;
#endif
      zz += hthick;
    } // End of loop over layers in a block
    zi     = zo;
    laymin = laymax;
    if (fabs(thickTot-layerThick_[i]) < 0.00001) {
    } else if (thickTot > layerThick_[i]) {
      edm::LogError("HGCalGeom") << "Thickness of the partition " 
				 << layerThick_[i] << " is smaller than "
				 << thickTot << ": thickness of all its "
				 << "components **** ERROR ****";
    } else if (thickTot < layerThick_[i]) {
      edm::LogWarning("HGCalGeom") << "Thickness of the partition " 
				   << layerThick_[i] << " does not match with "
				   << thickTot << " of the components";
    }
  }   // End of loop over blocks
}

double DDHGCalEEAlgo::rMax(double z) {
  double r(0);
#ifdef EDM_ML_DEBUG
  unsigned int ik(0);
#endif
  for (unsigned int k=0; k<slopeT_.size(); ++k) {
    if (z < zFront_[k]) break;
    r  = rMaxFront_[k] + (z - zFront_[k]) * slopeT_[k];
#ifdef EDM_ML_DEBUG
    ik = k;
#endif
  }
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo:rMax : " << z << ":" << ik 
				<< ":" << r;
#endif
  return r;
}

void DDHGCalEEAlgo::positionSensitive(const DDLogicalPart& glog, double rin,
				      double rout, int layertype,
				      DDCompactView& cpv) {
  static const double sqrt3 = std::sqrt(3.0);
  double r    = 0.5*(waferSize_ + waferSepar_);
  double R    = 2.0*r/sqrt3;
  double dy   = 0.75*R;
  int    N    = (int)(0.5*rout/r) + 2;
  double xc[6], yc[6];
#ifdef EDM_ML_DEBUG
  int    ium(0), ivm(0), iumAll(0), ivmAll(0), kount(0), ntot(0), nin(0);
  std::vector<int>  ntype(6,0);
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: " << glog.ddname() 
				<< " rout " << rout << " N " 
				<< N << " for maximum u, v";
#endif
  for (int u = -N; u <= N; ++u) {
    int iu = std::abs(u);
    for (int v = -N; v <= N; ++v) {
      int iv = std::abs(v);
      int nr = 2*v;
      int nc =-2*u+v;
      double xpos = nc*r;
      double ypos = nr*dy;
      xc[0] = xpos+r;  yc[0] = ypos+0.5*R;
      xc[1] = xpos;    yc[1] = ypos+R;
      xc[2] = xpos-r;  yc[2] = ypos+0.5*R;
      xc[3] = xpos-r;  yc[3] = ypos-0.5*R;
      xc[4] = xpos;    yc[4] = ypos-R;
      xc[5] = xpos+r;  yc[5] = ypos-0.5*R;
      bool cornerOne(false), cornerAll(true);
      for (int k=0; k<6; ++k) {
	double rpos = std::sqrt(xc[k]*xc[k]+yc[k]*yc[k]);
	if (rpos >= rin && rpos <= rout) cornerOne = true;
	else                             cornerAll = false;
      }
#ifdef EDM_ML_DEBUG
      ++ntot;
#endif
      if (cornerOne) {
	int copy = iv*100 + iu;
	if (u < 0) copy += 10000;
	if (v < 0) copy += 100000;
#ifdef EDM_ML_DEBUG
	if (iu > ium) ium = iu;
	if (iv > ivm) ivm = iv;
	kount++;
	if (copies_.count(copy) == 0) copies_.insert(copy);
#endif
	if (cornerAll) {
#ifdef EDM_ML_DEBUG
	  if (iu > iumAll) iumAll = iu;
	  if (iv > ivmAll) ivmAll = iv;
	  ++nin;
#endif
	  double rpos = std::sqrt(xpos*xpos+ypos*ypos);
	  DDTranslation tran(xpos, ypos, 0.0);
	  DDRotation rotation;
	  int type = (rpos < rMaxFine_) ? 0 : ((rpos < rMinThick_) ? 1 : 2);
	  if (layertype > 1) type += 3;
	  DDName name = DDName(DDSplit(wafers_[type]).first, 
			       DDSplit(wafers_[type]).second);
	  cpv.position(name, glog.ddname(), copy, tran, rotation);
#ifdef EDM_ML_DEBUG
	  ++ntype[type];
	  edm::LogVerbatim("HGCalGeom") << " DDHGCalEEAlgo: " << name 
					<< " number " << copy
					<< " positioned in " << glog.ddname()
					<< " at " << tran 
					<< " with " << rotation;
#endif
	}
      }
    }
  }
#ifdef EDM_ML_DEBUG
  edm::LogVerbatim("HGCalGeom") << "DDHGCalEEAlgo: Maximum # of u " << ium 
				<< ":" << iumAll << " # of v " << ivm << ":" 
				<< ivmAll << " and " << nin << ":" << kount 
				<< ":" << ntot << " wafers (" << ntype[0] 
				<< ":" << ntype[1] << ":" << ntype[2] << ":"
				<< ntype[3] << ":" << ntype[4] << ":"
				<< ntype[5] << ") for " << glog.ddname() 
				<< " R " << rin << ":" << rout;
#endif
}
