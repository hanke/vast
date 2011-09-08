#include "QOrientationHandler.hpp"


namespace isis {
    
namespace viewer {
namespace qt {
  
    
util::fvector4 QOrienationHandler::mapCoordsToOrientation( const util::fvector4 &coords, const boost::shared_ptr< ImageHolder > image, PlaneOrientation orientation, bool back, bool absolute)
{
    using namespace boost::numeric::ublas;
    matrix<float> transformMatrix = identity_matrix<float>( 4, 4 );
    matrix<float> finMatrix = identity_matrix<float>(4,4);
    vector<float> vec = vector<float>(4);
    vector<float> finVec = vector<float>(4);
    for( size_t i = 0; i< 4 ; i++ ) {
	vec(i) = coords[i];
    }
    switch ( orientation ) {
    case axial:
	    /*setup axial matrix
	    *-1  0  0
	    * 0 -1  0
	    * 0  0  1
	    */
	    transformMatrix( 0, 0 ) = -1;
	    transformMatrix( 1, 1 ) = 1;
	    break;
    case sagittal:
	    /*setup sagittal matrix
	    * 0  1  0
	    * 0  0  1
	    * 1  0  0
	    */
	    transformMatrix( 0, 0 ) = 0;
	    transformMatrix( 2, 0 ) = 1;
	    transformMatrix( 0, 1 ) = 1;
	    transformMatrix( 2, 2 ) = 0;
	    transformMatrix( 1, 2 ) = -1;
	    transformMatrix( 1, 1 ) = 0;
	    break;
    case coronal:
	    /*setup coronal matrix
	    * -1  0  0
	    *  0  0  1
	    *  0  1  0
	    */

	    transformMatrix( 0, 0 ) = -1;
	    transformMatrix( 1, 1 ) = 0;
	    transformMatrix( 2, 2 ) = 0;
	    transformMatrix( 2, 1 ) = 1;
	    transformMatrix( 1, 2 ) = -1;
	    break;
    }
    if(back) {
	finVec = prod( trans(prod(  transformMatrix, image->getNormalizedImageOrientation() )), vec );
    } else {
	finVec = prod( prod( transformMatrix, image->getNormalizedImageOrientation()) , vec);
    }
    if( absolute ) {
	return util::fvector4( fabs(finVec(0)), fabs(finVec(1)), fabs(finVec(2)), fabs(finVec(3)));
    } else {
	return util::fvector4( finVec(0), finVec(1), finVec(2), finVec(3) );
    }
  
}

util::ivector4 QOrienationHandler::getMappedCoords(const boost::shared_ptr< ImageHolder > image, const util::ivector4 &coords)
{
    using namespace boost::numeric::ublas;
    vector<int16_t> vec = vector<int16_t>(4);
    vector<int16_t> vec1 = vector<int16_t>(4);
    util::ivector4 retVec;
    for( size_t i= 0; i< 4;i++) {
	vec(i) = coords[i];
    }
    vec1 = prod(image->getNormalizedImageOrientation(), vec );
    for ( size_t i= 0; i< 4;i++) {
	retVec[i] = abs(vec1(i));
    }
    return retVec;
}



util::fvector4 QOrienationHandler::getScalingAndOffset(const boost::shared_ptr< ImageHolder > image, const size_t& w, const size_t& h, PlaneOrientation orientation)
{
    util::fvector4 retVec;
    util::ivector4 mappedSize = QOrienationHandler::mapCoordsToOrientation( image->getImageSize(), image, orientation );
    util::fvector4 mappedScaling = QOrienationHandler::mapCoordsToOrientation( image->getImage()->getPropertyAs<util::fvector4>("voxelSize"), image, orientation );
    util::fvector4 physicalSize = mappedScaling * mappedSize; 

    float scalew = w / float(mappedSize[0]);
    float scaleh = h / float(mappedSize[1]);
    
    float normh = scalew < scaleh ? scalew / scaleh : 1;
    float normw = scalew > scaleh ? scaleh / scalew : 1;
    retVec[0] = scalew * normw;
    retVec[1] = scaleh * normh;
    retVec[2] = ( w - retVec[0] * mappedSize[0] ) / 2 ;
    retVec[3] = ( h - retVec[1] * mappedSize[1] ) / 2 ;
    return retVec;
    
}


QTransform QOrienationHandler::getTransform(const boost::shared_ptr< ImageHolder > image, const size_t& w, const size_t& h, PlaneOrientation orientation)
{
    util::ivector4 mappedSize = QOrienationHandler::mapCoordsToOrientation( image->getImageSize(), image, orientation );
    util::fvector4 flipVec = QOrienationHandler::mapCoordsToOrientation(util::fvector4(1,1,1), image, orientation, false, false );
    QTransform retTransform;
    retTransform.setMatrix(flipVec[0], 0, 0,
			   0, flipVec[1], 0,
			   0, 0, 1 );
    
    util::fvector4 scalingAndOffset = QOrienationHandler::getScalingAndOffset(image, w, h, orientation );
    retTransform.translate( flipVec[0] * scalingAndOffset[2], flipVec[1] * scalingAndOffset[3] );
    retTransform.scale(scalingAndOffset[0], scalingAndOffset[1] );
    retTransform.translate(  flipVec[0] < 0 ? -mappedSize[0] : 0, flipVec[1] < 0 ? -mappedSize[1] : 0 );

    return retTransform;
    
}



}}} // end namespace