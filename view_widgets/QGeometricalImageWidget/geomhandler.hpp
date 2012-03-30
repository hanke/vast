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
 * geomhandler.hpp
 *
 * Description:
 *
 *  Created on: Mar 27, 2012
 *      Author: tuerke
 ******************************************************************/
#ifndef VAST_GEOMHANDLER_HPP
#define VAST_GEOMHANDLER_HPP

#include "common.hpp"
#include "geometrical.hpp"
#include <QTransform>

namespace isis {
namespace viewer {
namespace widget {
namespace _internal {

const QTransform toISISAxial( -1, 0, 0, 1, 0, 0 );
const QTransform toISISSagittal( 1, 0, 0, -1, 0, 0 );
const QTransform toISISCoronal( -1, 0, 0, -1, 0, 0 );

const uint8_t rasteringFac = 10;

util::fvector4 getPhysicalBoundingBox( const ImageHolder::Vector images, const PlaneOrientation &orientation );

QTransform getQTransform( const ImageHolder::Pointer image, const PlaneOrientation &orientation );

QTransform getTransform2ISISSpace( const PlaneOrientation &orientation, const util::fvector4 &);

boost::numeric::ublas::matrix<float> extract2DMatrix( const boost::shared_ptr<ImageHolder> image, const PlaneOrientation &orientation, bool inv = true );

util::fvector4 mapPhysicalCoords2Orientation( const util::fvector4 &coords, const PlaneOrientation &orientation );

	

}}}}// end namespace


#endif // VAST_GEOMHANDLER_HPP