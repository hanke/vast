/****************************************************************
 *
 * <Copyright information>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * Author: Erik Tuerke, tuerke@cbs.mpg.de
 *
 * geomhandler.cpp
 *
 * Description:
 *
 *  Created on: Mar 27, 2012
 *      Author: tuerke
 ******************************************************************/

#include "geomhandler.hpp"
#include "geometrical.hpp"
#include <imageholder.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <Adapter/qmatrixconversion.hpp>
#include <DataStorage/common.hpp>

namespace isis {
namespace viewer {
namespace widget {
namespace _internal {

geometrical::BoundingBoxType getVoxelBoundingBox ( const ImageHolder::Vector& images )
{
	geometrical::BoundingBoxType retBox;
	for( unsigned short i = 0; i< 3;i++ ) {
		retBox[i].first = 0;
		retBox[i].second = -std::numeric_limits<float>::max();
	}
	BOOST_FOREACH(const ImageHolder::Vector::const_reference image, images ) {
		const util::ivector4 size = mapCoordsToOrientation(image->getISISImage()->getSizeAsVector(), image->getImageProperties().latchedOrientation, axial, false, true);
		const util::ivector4 vsize = mapCoordsToOrientation(image->getImageProperties().voxelSize, image->getImageProperties().latchedOrientation, axial, false, true);
		for( unsigned short i = 0; i<3;i++ ) {
			if( size[i] > retBox[i].second ) {
				retBox[i].second = size[i] * vsize[i];
			}
		}
	}
	return retBox;
}


	
util::fvector4 getPhysicalBoundingBox ( const ImageHolder::Vector &images, const PlaneOrientation &orientation, const bool &latched )
{
	geometrical::BoundingBoxType currentBoundingBox;
	if( latched ) {
		currentBoundingBox = getVoxelBoundingBox( images );
	} else {
		currentBoundingBox = geometrical::getPhysicalBoundingBox(images);
	}

	util::fvector4 ret;
	switch(orientation ) {
		case axial:
			ret = util::fvector4( 	currentBoundingBox[0].first,
									currentBoundingBox[1].first,
									fabs( currentBoundingBox[0].second - currentBoundingBox[0].first ),
									fabs( currentBoundingBox[1].second - currentBoundingBox[1].first ) );
			break;
		case sagittal:
			ret = util::fvector4(	currentBoundingBox[1].first,
									currentBoundingBox[2].first,
									fabs(  currentBoundingBox[1].second - currentBoundingBox[1].first ),
									fabs(  currentBoundingBox[2].second - currentBoundingBox[2].first ) );
			break;
		case coronal:
			ret = util::fvector4(	currentBoundingBox[0].first,
									currentBoundingBox[2].first,
									fabs(  currentBoundingBox[0].second - currentBoundingBox[0].first ),
									fabs(  currentBoundingBox[2].second - currentBoundingBox[2].first ) );
			break;
		case not_specified:
			break;
	}
	return ret * rasteringFac;
}

QTransform getQTransform ( const ImageHolder::Pointer image, const PlaneOrientation& orientation, bool latched )
{
	
	const util::FixedMatrix<qreal,2,2> mat = extract2DMatrix(image, orientation, latched, false );
	
	util::FixedVector<qreal,2> vc;
	util::fvector4 mapped_voxelSize = mapCoordsToOrientation(image->getImageProperties().voxelSize, image->getImageProperties().latchedOrientation, orientation, false, true) * rasteringFac;
	util::FixedVector<qreal,2> _vc;
	util::fvector4 _io;

	if(latched) {
		_io.fill(0);
		_vc.fill(0);
	} else {
		const uint16_t slice = mapCoordsToOrientation(image->getImageProperties().voxelCoords, image->getImageProperties().latchedOrientation, orientation, false, true)[2];
		const util::ivector4 mappedCoords = mapCoordsToOrientation(util::ivector4(0,0,slice), image->getImageProperties().latchedOrientation, orientation, true, true);
		_io = image->getISISImage()->getPhysicalCoordsFromIndex(mappedCoords) * rasteringFac ;
		vc[0] = mapped_voxelSize[0] / 2.0; vc[1] = mapped_voxelSize[1] / 2.0;
		_vc = mat.dot(vc);
	}
	switch(orientation) {
		case axial:{
			QTransform tr1;
			return QTransform(QTransform(mat.elem(0,0), mat.elem(0,1), mat.elem(1,0), mat.elem(1,1),0,0)
					* tr1.translate(_io[0] - _vc[0], _io[1] - _vc[1])).scale(mapped_voxelSize[0], mapped_voxelSize[1]);
			break;
		}
		case sagittal:{
			QTransform tr1;
			return QTransform(QTransform(mat.elem(0,0), mat.elem(0,1), mat.elem(1,0), mat.elem(1,1),0,0)
					* tr1.translate(_io[1] - _vc[0], _io[2] - _vc[1])).scale(mapped_voxelSize[0], mapped_voxelSize[1]);
			break;
		}
		case coronal:{
			QTransform tr1;
			return QTransform(QTransform(mat.elem(0,0), mat.elem(0,1), mat.elem(1,0), mat.elem(1,1),0,0)
					* tr1.translate(_io[0] - _vc[0], _io[2] - _vc[1])).scale(mapped_voxelSize[0], mapped_voxelSize[1]);
			break;
		}
	}
	return QTransform();
}

QTransform getTransform2ISISSpace ( const PlaneOrientation& orientation, const util::fvector4 &bb )
{
	QTransform retTransform;
	switch ( orientation ) {
		case axial:
			retTransform = QTransform( -1, 0, 0, 1, bb[2] + (bb[0]*2), 0 );
			break;
		case sagittal:
			retTransform = QTransform( 1, 0, 0, -1, 0, bb[3] + (bb[1]*2) );
			break;
		case coronal:
			retTransform = QTransform( -1, 0, 0, -1, bb[2] + (bb[0]*2), bb[3] + (bb[1]*2) );
			break;
		case not_specified:
			break;
	}
	return retTransform;
}

util::FixedMatrix<qreal,2,2> extract2DMatrix ( const boost::shared_ptr<ImageHolder> image, const PlaneOrientation& orientation, bool latched, bool inverse )
{

	util::FixedMatrix<qreal,2,2> retMatrix;
	util::Matrix4x4<qreal> latchedOrientation_abs;
	
	for( unsigned short i = 0; i<3; i++ ) {
		for( unsigned short j = 0; j<3; j++ ) {
			latchedOrientation_abs.elem(i,j) = fabs(image->getImageProperties().latchedOrientation.elem(i,j));
		}
	}
	util::Matrix4x4<qreal> invLatchedOrientation;
	bool invOk;
	if( inverse ) {
		invLatchedOrientation = latchedOrientation_abs.inverse(invOk);
		if( !invOk ) {
			LOG( Dev, warning ) << "Failed to usee inverse to extract 2D matrix for image " << image->getImageProperties().filePath
						<< "! Falling back to transposed.";
		}
	}
	if( !inverse || !invOk ) {
		invLatchedOrientation = latchedOrientation_abs.transpose();
	}
	const util::Matrix4x4<qreal> mat = latched ? image->getImageProperties().latchedOrientation.dot(invLatchedOrientation) : image->getImageProperties().orientation.dot(invLatchedOrientation);
	switch(orientation) {
		case axial:
			retMatrix.elem(0,0) = mat.elem(0,0);
			retMatrix.elem(0,1) = mat.elem(0,1);
			retMatrix.elem(1,0) = mat.elem(1,0);
			retMatrix.elem(1,1) = mat.elem(1,1);
			break;
		case sagittal:
			retMatrix.elem(0,0) = mat.elem(1,1);
			retMatrix.elem(1,0) = mat.elem(2,1);
			retMatrix.elem(0,1) = mat.elem(1,2);
			retMatrix.elem(1,1) = mat.elem(2,2);
			break;
		case coronal:
			retMatrix.elem(0,0) = mat.elem(0,0);
			retMatrix.elem(1,0) = mat.elem(2,0);
			retMatrix.elem(0,1) = mat.elem(0,2);
			retMatrix.elem(1,1) = mat.elem(2,2);
			break;
		case not_specified:
			break;
	}
	return retMatrix;
	
}

util::fvector4 mapPhysicalCoords2Orientation ( const util::fvector4& coords, const PlaneOrientation& orientation )
{
	util::fvector4 retCoords = coords;
	switch( orientation ) {
		case axial:
			retCoords = coords;
			break;
		case sagittal:
			retCoords[0] = coords[1];
			retCoords[1] = coords[2];
			retCoords[2] = coords[0];
			break;
		case coronal:
			retCoords[1] = coords[2];
			retCoords[2] = coords[1];
			break;
		case not_specified:
			break;
	}
	return retCoords;
}

void zoomBoundingBox ( util::fvector4& boundingBox, util::FixedVector<float,2> &translation, const util::fvector4& physCoord, const float &zoom, const PlaneOrientation &orientation, const bool &translate )
{
	const util::fvector4 oldBoundingBox = boundingBox;
	if( translate ) {
		const float centerX = boundingBox[0] + ( boundingBox[2] / 2.);
		const float centerY = boundingBox[1] + ( boundingBox[3] / 2.);

		const util::fvector4 mappedPhysicalCoords = mapPhysicalCoords2Orientation( physCoord, orientation ) * rasteringFac;
		const float diffX = ((centerX - mappedPhysicalCoords[0]) / (boundingBox[2] / 2.));
		const float diffY = ((centerY - mappedPhysicalCoords[1]) / (boundingBox[3] / 2.));
		translation[0] = diffX * ( (boundingBox[2] - (boundingBox[2] / zoom)) / 2.);
		translation[1] = diffY * ( (boundingBox[3] - (boundingBox[3] / zoom)) / 2.);
	}
	boundingBox[2] /= zoom;
	boundingBox[3] /= zoom;
	//translate bounding box
	boundingBox[0] += (oldBoundingBox[2] - boundingBox[2] ) / 2.;
	boundingBox[1] += (oldBoundingBox[3] - boundingBox[3] ) / 2.;
}


}}}}