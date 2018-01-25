/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#ifndef TUDAT_CREATEOBSERVATIONMODEL_H
#define TUDAT_CREATEOBSERVATIONMODEL_H

#include <map>

#include <boost/function.hpp>
#include <boost/make_shared.hpp>


#include "Tudat/Astrodynamics/ObservationModels/observationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/linkTypeDefs.h"
#include "Tudat/SimulationSetup/EstimationSetup/createLightTimeCorrection.h"
#include "Tudat/Astrodynamics/ObservationModels/nWayRangeObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/oneWayRangeObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/oneWayDopplerObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/twoWayDopplerObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/oneWayDifferencedRangeRateObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/angularPositionObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/positionObservationModel.h"
#include "Tudat/Astrodynamics/ObservationModels/observationSimulator.h"
#include "Tudat/Astrodynamics/ObservationModels/observationViabilityCalculator.h"
#include "Tudat/SimulationSetup/EnvironmentSetup/body.h"
#include "Tudat/SimulationSetup/EstimationSetup/createLightTimeCalculator.h"


namespace tudat
{

namespace observation_models
{

//! Base class to define settings for creation of an observation bias model.
/*!
 *  Base class to define settings for creation of an observation bias model. For each specific bias type, a derived class
 *  is to be implemented, in which the specific properties of the bias model are given
 */
class ObservationBiasSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param observationBiasType Type of bias model that is to be created.
     */
    ObservationBiasSettings(
            const observation_models::ObservationBiasTypes observationBiasType ):
        observationBiasType_( observationBiasType ){ }

    //! Destructor
    virtual ~ObservationBiasSettings( ){ }

    //! Type of bias model that is to be created.
    observation_models::ObservationBiasTypes observationBiasType_;
};

//! Class for defining settings for the creation of a multiple biases for a single observable
class MultipleObservationBiasSettings: public ObservationBiasSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param biasSettingsList List of settings for bias objects that are to be created.
     */
    MultipleObservationBiasSettings(
            const std::vector< boost::shared_ptr< ObservationBiasSettings > > biasSettingsList ):
        ObservationBiasSettings( multiple_observation_biases ),
        biasSettingsList_( biasSettingsList ){ }

    //! Destructor
    ~MultipleObservationBiasSettings( ){ }

    //! List of settings for bias objects that are to be created.
    std::vector< boost::shared_ptr< ObservationBiasSettings > > biasSettingsList_;
};

//! Class for defining settings for the creation of a constant absolute or relative observation bias model
class ConstantObservationBiasSettings: public ObservationBiasSettings
{
public:

    //! Constuctor
    /*!
     * Constuctor
     * \param observationBias Constant bias that is to be added to the observable. The size of this vector must be equal to the
     * size of the observable to which it is assigned.
     * \param useAbsoluteBias Boolean to denote whether an absolute or relative bias is to be created.
     */
    ConstantObservationBiasSettings(
            const Eigen::VectorXd& observationBias,
            const bool useAbsoluteBias ):
        ObservationBiasSettings( ( useAbsoluteBias == true ) ? ( constant_absolute_bias ) : ( constant_relative_bias ) ),
        observationBias_( observationBias ), useAbsoluteBias_( useAbsoluteBias ){ }

    //! Destructor
    ~ConstantObservationBiasSettings( ){ }

    //! Constant bias that is to be added to the observable.
    /*!
     *  Constant bias that is to be added to the observable. The size of this vector must be equal to the
     *  size of the observable to which it is assigned.
     */
    Eigen::VectorXd observationBias_;

    //! Boolean to denote whether an absolute or relative bias is to be created.
    bool useAbsoluteBias_;
};

//! Class for defining settings for the creation of an arc-wise constant absolute or relative observation bias model
class ArcWiseConstantObservationBiasSettings: public ObservationBiasSettings
{
public:

    //! Constuctor
    /*!
     * Constuctor
     * \param arcStartTimes Start times for arcs in which biases (observationBiases) are used
     * \param observationBiases List of observation biases per arc
     * \param linkEndForTime Link end at which time is to be evaluated to determine current time (and current arc)
     * \param useAbsoluteBias Boolean to denote whether an absolute or relative bias is to be created.
     */
    ArcWiseConstantObservationBiasSettings(
            const std::vector< double >& arcStartTimes,
            const std::vector< Eigen::VectorXd >& observationBiases,
            const LinkEndType linkEndForTime,
            const bool useAbsoluteBias ):
        ObservationBiasSettings( ( useAbsoluteBias == true ) ?
                                     ( arc_wise_constant_absolute_bias ) : ( arc_wise_constant_relative_bias ) ),
        arcStartTimes_( arcStartTimes ), observationBiases_( observationBiases ), linkEndForTime_( linkEndForTime ),
        useAbsoluteBias_( useAbsoluteBias ){ }

    //! Constuctor
    /*!
     * Constuctor
     * \param observationBiases Map of observation biases per arc, with bias as map value, and arc start time as map key
     * \param linkEndForTime Link end at which time is to be evaluated to determine current time (and current arc)
     * \param useAbsoluteBias Boolean to denote whether an absolute or relative bias is to be created.
     */
    ArcWiseConstantObservationBiasSettings(
            const std::map< double, Eigen::VectorXd >& observationBiases,
            const LinkEndType linkEndForTime,
            const bool useAbsoluteBias  ):
        ObservationBiasSettings( ( useAbsoluteBias == true ) ?
                                     ( arc_wise_constant_absolute_bias ) : ( arc_wise_constant_relative_bias ) ),
        arcStartTimes_( utilities::createVectorFromMapKeys( observationBiases ) ),
        observationBiases_( utilities::createVectorFromMapValues( observationBiases ) ), linkEndForTime_( linkEndForTime ),
        useAbsoluteBias_( useAbsoluteBias ){ }

    //! Destructor
    ~ArcWiseConstantObservationBiasSettings( ){ }

    //! Start times for arcs in which biases (observationBiases) are used
    std::vector< double > arcStartTimes_;

    //! List of observation biases per arc
    std::vector< Eigen::VectorXd > observationBiases_;

    //! Link end at which time is to be evaluated to determine current time (and current arc)
    LinkEndType linkEndForTime_;

    //! Boolean to denote whether an absolute or relative bias is to be created.
    bool useAbsoluteBias_;
};

//! Class used for defining the settings for an observation model that is to be created.
/*!
 * Class used for defining the settings for an observation model that is to be created. This class allows the type, light-time
 * corrections and bias for the observation to be set. For observation models that require additional information (e.g.
 * integration time, retransmission time, etc.), a specific derived class must be implemented.
 */
class ObservationSettings
{
public:


    //! Constructor
    /*!
     * Constructor (single light-time correction)
     * \param observableType Type of observation model that is to be created
     * \param lightTimeCorrections Settings for a single light-time correction that is to be used for teh observation model
     * (NULL if none)
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NULL)
     */
    ObservationSettings(
            const observation_models::ObservableType observableType,
            const boost::shared_ptr< LightTimeCorrectionSettings > lightTimeCorrections,
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        observableType_( observableType ),
        biasSettings_( biasSettings )
    {
        if( lightTimeCorrections != NULL )
        {
            lightTimeCorrectionsList_.push_back( lightTimeCorrections );
        }
    }

    //! Constructor
    /*!
     * Constructor (multiple light-time correction)
     * \param observableType Type of observation model that is to be created
     * \param lightTimeCorrectionsList List of settings for a single light-time correction that is to be used for the observation
     * model
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NULL)
     */
    ObservationSettings(
            const observation_models::ObservableType observableType,
            const std::vector< boost::shared_ptr< LightTimeCorrectionSettings > > lightTimeCorrectionsList =
            std::vector< boost::shared_ptr< LightTimeCorrectionSettings > >( ),
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        observableType_( observableType ),lightTimeCorrectionsList_( lightTimeCorrectionsList ),
        biasSettings_( biasSettings ){ }

    //! Destructor
    virtual ~ObservationSettings( ){ }

    //! Type of observation model that is to be created
    observation_models::ObservableType observableType_;

    //! List of settings for a single light-time correction that is to be used for the observation model
    std::vector< boost::shared_ptr< LightTimeCorrectionSettings > > lightTimeCorrectionsList_;

    //! Settings for the observation bias model that is to be used (default none: NULL)
    boost::shared_ptr< ObservationBiasSettings > biasSettings_;
};

//! Enum defining all possible types of proper time rate computations in one-way Doppler
enum DopplerProperTimeRateType
{
    custom_doppler_proper_time_rate,
    direct_first_order_doppler_proper_time_rate
};

//! Base class to define the settings for proper time rate (at a single link end) in one-way Doppler mode.
class DopplerProperTimeRateSettings
{
public:
    DopplerProperTimeRateSettings( const DopplerProperTimeRateType dopplerProperTimeRateType ):
        dopplerProperTimeRateType_( dopplerProperTimeRateType ){ }

    virtual ~DopplerProperTimeRateSettings( ){ }

    DopplerProperTimeRateType dopplerProperTimeRateType_;
};

//! Class to define the settings for first-order, single body, proper time rate (at a single link end) in one-way Doppler mode.
class DirectFirstOrderDopplerProperTimeRateSettings: public DopplerProperTimeRateSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param centralBodyName Name of central body, fromw which the mass monopole is retrieved to compute the proper time rate,
     * and w.r.t. which the velocity of the point at which proper time rate is computed is taken
     */
    DirectFirstOrderDopplerProperTimeRateSettings(
            const std::string centralBodyName ):
        DopplerProperTimeRateSettings( direct_first_order_doppler_proper_time_rate ),
        centralBodyName_( centralBodyName ){ }

    //! Destructor.
    ~DirectFirstOrderDopplerProperTimeRateSettings( ){ }

    //! Name of central body
    /*!
     * Name of central body, fromw which the mass monopole is retrieved to compute the proper time rate,
     * and w.r.t. which the velocity of the point at which proper time rate is computed is taken
     */
    std::string centralBodyName_;
};

//! Class to define the settings for one-way Doppler observable
class OneWayDopplerObservationSettings: public ObservationSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param lightTimeCorrections Settings for a single light-time correction that is to be used for teh observation model
     * (NULL if none)
     * \param transmitterProperTimeRateSettings Settings for proper time rate at transmitter
     * \param receiverProperTimeRateSettings Settings for proper time rate at receiver
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NUL
     */
    OneWayDopplerObservationSettings(
            const boost::shared_ptr< LightTimeCorrectionSettings > lightTimeCorrections,
            const boost::shared_ptr< DopplerProperTimeRateSettings > transmitterProperTimeRateSettings = NULL,
            const boost::shared_ptr< DopplerProperTimeRateSettings > receiverProperTimeRateSettings = NULL,
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( one_way_doppler, lightTimeCorrections, biasSettings ),
        transmitterProperTimeRateSettings_( transmitterProperTimeRateSettings ),
        receiverProperTimeRateSettings_( receiverProperTimeRateSettings ){ }

    //! Constructor
    /*!
     * Constructor
     * \param lightTimeCorrectionsList List of settings for a single light-time correction that is to be used for teh observation
     * model (empty if none)
     * \param transmitterProperTimeRateSettings Settings for proper time rate at transmitter
     * \param receiverProperTimeRateSettings Settings for proper time rate at receiver
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NUL
     */
    OneWayDopplerObservationSettings(
            const std::vector< boost::shared_ptr< LightTimeCorrectionSettings > > lightTimeCorrectionsList =
            std::vector< boost::shared_ptr< LightTimeCorrectionSettings > >( ),
            const boost::shared_ptr< DopplerProperTimeRateSettings > transmitterProperTimeRateSettings = NULL,
            const boost::shared_ptr< DopplerProperTimeRateSettings > receiverProperTimeRateSettings = NULL,
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( one_way_doppler, lightTimeCorrectionsList, biasSettings ),
        transmitterProperTimeRateSettings_( transmitterProperTimeRateSettings ),
        receiverProperTimeRateSettings_( receiverProperTimeRateSettings ){ }

    //! Destructor
    ~OneWayDopplerObservationSettings( ){ }

    //! Settings for proper time rate at transmitter
    boost::shared_ptr< DopplerProperTimeRateSettings > transmitterProperTimeRateSettings_;

    //! Settings for proper time rate at receiver
    boost::shared_ptr< DopplerProperTimeRateSettings > receiverProperTimeRateSettings_;
};



//! Class to define the settings for one-way Doppler observable
class TwoWayDopplerObservationSettings: public ObservationSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param uplinkOneWayDopplerSettings Settings for the one-way Doppler model of the uplink
     * \param downlinkOneWayDopplerSettings Settings for the one-way Doppler model of the downlink
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NUL
     */
    TwoWayDopplerObservationSettings(
            const boost::shared_ptr< OneWayDopplerObservationSettings > uplinkOneWayDopplerSettings,
            const boost::shared_ptr< OneWayDopplerObservationSettings > downlinkOneWayDopplerSettings,
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( two_way_doppler, boost::shared_ptr< LightTimeCorrectionSettings >( ), biasSettings ),
        uplinkOneWayDopplerSettings_( uplinkOneWayDopplerSettings ),
        downlinkOneWayDopplerSettings_( downlinkOneWayDopplerSettings ){ }

    //! Destructor
    ~TwoWayDopplerObservationSettings( ){ }

    //! Settings for the one-way Doppler model of the uplink
    boost::shared_ptr< OneWayDopplerObservationSettings > uplinkOneWayDopplerSettings_;

    //! Settings for the one-way Doppler model of the downlink
    boost::shared_ptr< OneWayDopplerObservationSettings > downlinkOneWayDopplerSettings_;
};




//! Class to define the settings for one-way differenced range-rate (e.g. closed-loop Doppler) observable
class OneWayDifferencedRangeRateObservationSettings: public ObservationSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param integrationTimeFunction Function that returns the integration time of observable as a function of time
     * \param lightTimeCorrections Settings for a single light-time correction that is to be used for teh observation model
     * (NULL if none)
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NULL)
     */
    OneWayDifferencedRangeRateObservationSettings(
            const boost::function< double( const double ) > integrationTimeFunction,
            const boost::shared_ptr< LightTimeCorrectionSettings > lightTimeCorrections,
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( one_way_differenced_range, lightTimeCorrections, biasSettings ),
        integrationTimeFunction_( integrationTimeFunction ){ }

    //! Constructor
    /*!
     * Constructor
     * \param integrationTimeFunction Function that returns the integration time of observable as a function of time
     * \param lightTimeCorrectionsList List of ettings for a single light-time correction that is to be used for teh observation model
     * (empty if none)
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NULL)
     */
    OneWayDifferencedRangeRateObservationSettings(
            const boost::function< double( const double ) > integrationTimeFunction,
            const std::vector< boost::shared_ptr< LightTimeCorrectionSettings > > lightTimeCorrectionsList =
            std::vector< boost::shared_ptr< LightTimeCorrectionSettings > >( ),
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( one_way_differenced_range, lightTimeCorrectionsList, biasSettings ),
        integrationTimeFunction_( integrationTimeFunction ){ }

    //! Destructor
    ~OneWayDifferencedRangeRateObservationSettings( ){ }

    //! Function that returns the integration time of observable as a function of time
    const boost::function< double( const double ) > integrationTimeFunction_;

};


//! Class to define the settings for one-way differenced range-rate (e.g. closed-loop Doppler) observable
class NWayRangeObservationSettings: public ObservationSettings
{
public:

    //! Constructor
    /*!
     * Constructor
     * \param oneWayRangeObsevationSettings List of settings for one-way observables that make up n-way link (each must be for
     * one_way_range_
     * \param retransmissionTimesFunction Function that returns the retransmission delay time of the signal as a function of
     * observation timew.
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NULL)
     */
    NWayRangeObservationSettings(
            const std::vector< boost::shared_ptr< ObservationSettings > > oneWayRangeObsevationSettings,
            const boost::function< std::vector< double >( const double ) > retransmissionTimesFunction =
            boost::function< std::vector< double >( const double  ) >( ),
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( n_way_range, std::vector< boost::shared_ptr< LightTimeCorrectionSettings > >( ), biasSettings ),
        oneWayRangeObsevationSettings_( oneWayRangeObsevationSettings ),
        retransmissionTimesFunction_( retransmissionTimesFunction ){ }

    //! Constructor
    /*!
     * Constructor for same light-time corrections per link
     * \param lightTimeCorrections Settings for a single light-time correction that is to be used for teh observation model
     * (NULL if none)
     * \param numberOfLinkEnds Number of link ends in observable (equal to n+1 for 'n'-way observable)
     * \param retransmissionTimesFunction Function that returns the retransmission delay time of the signal as a function of
     * observation timew.
     * \param biasSettings Settings for the observation bias model that is to be used (default none: NULL)
     */
    NWayRangeObservationSettings(
            const boost::shared_ptr< LightTimeCorrectionSettings > lightTimeCorrections,
            const int numberOfLinkEnds,
            const boost::function< std::vector< double >( const double ) > retransmissionTimesFunction =
            boost::function< std::vector< double >( const double  ) >( ),
            const boost::shared_ptr< ObservationBiasSettings > biasSettings = NULL ):
        ObservationSettings( n_way_range, std::vector< boost::shared_ptr< LightTimeCorrectionSettings > >( ), biasSettings ),
        retransmissionTimesFunction_( retransmissionTimesFunction )
    {
        for( int i = 0; i < numberOfLinkEnds - 1; i++ )
        {
            oneWayRangeObsevationSettings_.push_back( boost::make_shared< ObservationSettings >(
                                                          one_way_range, lightTimeCorrections ) );
        }
    }

    //! Destructor
    ~NWayRangeObservationSettings( ){ }

    std::vector< boost::shared_ptr< ObservationSettings > > oneWayRangeObsevationSettings_;

    //! Function that returns the integration time of observable as a function of time
    boost::function< std::vector< double >( const double ) > retransmissionTimesFunction_;

};

//! Function to create the proper time rate calculator for use in one-way Doppler
/*!
 *  Function to create the proper time rate calculator for use in one-way Doppler
 *  \param properTimeRateSettings Settings for proper time rate model
 *  \param linkEnds Link ends of one-way Doppler observation  model
 *  \param bodyMap List of body objects that constitutes the environment
 *  \param linkEndForCalculator Link end for which the proper time rate is to be computed
 *  \return Proper time rate calculator for use in one-way Doppler
 */
template< typename ObservationScalarType = double, typename TimeType = double >
boost::shared_ptr< DopplerProperTimeRateInterface > createOneWayDopplerProperTimeCalculator(
        boost::shared_ptr< DopplerProperTimeRateSettings > properTimeRateSettings,
        const LinkEnds& linkEnds,
        const simulation_setup::NamedBodyMap &bodyMap,
        const LinkEndType linkEndForCalculator )
{
    boost::shared_ptr< DopplerProperTimeRateInterface > properTimeRateInterface;

    // Check tyope of proper time rate model
    switch( properTimeRateSettings->dopplerProperTimeRateType_ )
    {
    case direct_first_order_doppler_proper_time_rate:
    {
        // Check input consistency
        boost::shared_ptr< DirectFirstOrderDopplerProperTimeRateSettings > directFirstOrderDopplerProperTimeRateSettings =
                boost::dynamic_pointer_cast< DirectFirstOrderDopplerProperTimeRateSettings >( properTimeRateSettings );
        if( directFirstOrderDopplerProperTimeRateSettings == NULL )
        {
            throw std::runtime_error( "Error when making DopplerProperTimeRateInterface, input type (direct_first_order_doppler_proper_time_rate) is inconsistent" );
        }
        else if( linkEnds.count( linkEndForCalculator ) == 0 )
        {
            std::string errorMessage = "Error when creating one-way Doppler proper time calculator, did not find link end " +
                    std::to_string( linkEndForCalculator );
            throw std::runtime_error( errorMessage );
        }
        else
        {
            if( bodyMap.at( directFirstOrderDopplerProperTimeRateSettings->centralBodyName_ )->getGravityFieldModel( ) == NULL )
            {
                throw std::runtime_error( "Error when making DirectFirstOrderDopplerProperTimeRateInterface, no gravity field found for " +
                                          directFirstOrderDopplerProperTimeRateSettings->centralBodyName_ );
            }
            else
            {
                // Retrieve gravitational parameter
                boost::function< double( ) > gravitationalParameterFunction =
                        boost::bind( &gravitation::GravityFieldModel::getGravitationalParameter,
                                     bodyMap.at( directFirstOrderDopplerProperTimeRateSettings->centralBodyName_ )->
                                     getGravityFieldModel( ) );

                // Create calculation object.
                LinkEndId referencePointId =
                        std::make_pair( directFirstOrderDopplerProperTimeRateSettings->centralBodyName_, "" );
                if( ( linkEnds.at( receiver ) != referencePointId ) && ( linkEnds.at( transmitter ) != referencePointId ) )
                {
                    properTimeRateInterface = boost::make_shared<
                            DirectFirstOrderDopplerProperTimeRateInterface >(
                                linkEndForCalculator, gravitationalParameterFunction,
                                directFirstOrderDopplerProperTimeRateSettings->centralBodyName_, unidentified_link_end,
                                getLinkEndCompleteEphemerisFunction< double, double >(
                                    std::make_pair( directFirstOrderDopplerProperTimeRateSettings->centralBodyName_, ""), bodyMap ) );
                }
                else
                {
                    throw std::runtime_error(
                                "Error, proper time reference point as link end not yet implemented for DopplerProperTimeRateInterface creation" );
                }
            }
        }
        break;
    }
    default:
        std::string errorMessage = "Error when creating one-way Doppler proper time calculator, did not recognize type " +
                std::to_string( properTimeRateSettings->dopplerProperTimeRateType_ );
        throw std::runtime_error( errorMessage );
    }
    return properTimeRateInterface;
}

//! Typedef of list of observation models per obserable type and link ends: note that ObservableType key must be consistent
//! with contents of ObservationSettings pointers. The ObservationSettingsMap may be used as well, which contains the same
//! type of information. This typedef, however, has some advantages in terms of book-keeping when creating observation models.
typedef std::map< ObservableType, std::map< LinkEnds, boost::shared_ptr< ObservationSettings > > > SortedObservationSettingsMap;

//! Typedef of list of observation models per link ends. Multiple observation models for a single set of link ends are allowed,
//! since this typedef represents a multimap.
typedef std::multimap< LinkEnds, boost::shared_ptr< ObservationSettings > > ObservationSettingsMap;

//! Function to create list of observation models sorted by observable type and link ends from list only sorted in link ends.
/*!
 * Function to create list of observation models sorted by observable type and link ends from list only sorted in link ends.
 * \param unsortedObservationSettingsMap List (multimap_) of observation models sorted link ends
 * \return List (map of maps) of observation models sorted by observable type and link ends
 */
SortedObservationSettingsMap convertUnsortedToSortedObservationSettingsMap(
        const ObservationSettingsMap& unsortedObservationSettingsMap );


//! Function to create an object that computes an observation bias
/*!
 *  Function to create an object that computes an observation bias, which can represent any type of system-dependent influence
 *  on the observed value (e.g. absolute bias, relative bias, clock drift, etc.)
 *  \param linkEnds Observation link ends for which the bias is to be created.
 *  \param observableType Observable type for which bias is to be created
 *  \param biasSettings Settings for teh observation bias that is to be created.
 *  \param bodyMap List of body objects that comprises the environment.
 *  \return Object that computes an observation bias according to requested settings.
 */
template< int ObservationSize = 1 >
boost::shared_ptr< ObservationBias< ObservationSize > > createObservationBiasCalculator(
        const LinkEnds linkEnds,
        const ObservableType observableType,
        const boost::shared_ptr< ObservationBiasSettings > biasSettings,
        const simulation_setup::NamedBodyMap &bodyMap )
{
    boost::shared_ptr< ObservationBias< ObservationSize > > observationBias;
    switch( biasSettings->observationBiasType_ )
    {
    case constant_absolute_bias:
    {
        // Check input consistency
        boost::shared_ptr< ConstantObservationBiasSettings > constantBiasSettings = boost::dynamic_pointer_cast<
                ConstantObservationBiasSettings >( biasSettings );
        if( constantBiasSettings == NULL )
        {
            throw std::runtime_error( "Error when making constant observation bias, settings are inconsistent" );
        }

        if( !constantBiasSettings->useAbsoluteBias_ )
        {
            throw std::runtime_error( "Error when making constant observation bias, class settings are inconsistent" );
        }

        // Check if size of bias is consistent with requested observable size
        if( constantBiasSettings->observationBias_.rows( ) != ObservationSize )
        {
            throw std::runtime_error( "Error when making constant observation bias, bias size is inconsistent" );
        }
        observationBias = boost::make_shared< ConstantObservationBias< ObservationSize > >(
                    constantBiasSettings->observationBias_ );
        break;
    }
    case arc_wise_constant_absolute_bias:
    {
        // Check input consistency
        boost::shared_ptr< ArcWiseConstantObservationBiasSettings > arcwiseBiasSettings = boost::dynamic_pointer_cast<
                ArcWiseConstantObservationBiasSettings >( biasSettings );
        if( arcwiseBiasSettings == NULL )
        {
            throw std::runtime_error( "Error when making arc-wise observation bias, settings are inconsistent" );
        }
        else if( !arcwiseBiasSettings->useAbsoluteBias_ )
        {
            throw std::runtime_error( "Error when making arc-wise observation bias, class contents are inconsistent" );
        }

        std::vector< Eigen::Matrix< double, ObservationSize, 1 > > observationBiases;
        for( unsigned int i = 0; i < arcwiseBiasSettings->observationBiases_.size( ); i++ )
        {
            // Check if size of bias is consistent with requested observable size
            if( arcwiseBiasSettings->observationBiases_.at( i ).rows( ) != ObservationSize )
            {
                throw std::runtime_error( "Error when making arc-wise observation bias, bias size is inconsistent" );
            }
            else
            {
                observationBiases.push_back( arcwiseBiasSettings->observationBiases_.at( i ) );
            }
        }
        observationBias = boost::make_shared< ConstantArcWiseObservationBias< ObservationSize > >(
                    arcwiseBiasSettings->arcStartTimes_, observationBiases,
                    observation_models::getLinkEndIndicesForLinkEndTypeAtObservable(
                        observableType, arcwiseBiasSettings->linkEndForTime_, linkEnds.size( ) ).at( 0 ) );
        break;
    }
    case constant_relative_bias:
    {
        // Check input consistency
        boost::shared_ptr< ConstantObservationBiasSettings > constantBiasSettings = boost::dynamic_pointer_cast<
                ConstantObservationBiasSettings >( biasSettings );
        if( constantBiasSettings == NULL )
        {
            throw std::runtime_error( "Error when making constant relative observation bias, settings are inconsistent" );
        }

        if( constantBiasSettings->useAbsoluteBias_ )
        {
            throw std::runtime_error( "Error when making constant relative observation bias, class settings are inconsistent" );
        }

        // Check if size of bias is consistent with requested observable size
        if( constantBiasSettings->observationBias_.rows( ) != ObservationSize )
        {
            throw std::runtime_error( "Error when making constant relative observation bias, bias size is inconsistent" );
        }
        observationBias = boost::make_shared< ConstantRelativeObservationBias< ObservationSize > >(
                    constantBiasSettings->observationBias_ );
        break;
    }
    case arc_wise_constant_relative_bias:
    {
        // Check input consistency
        boost::shared_ptr< ArcWiseConstantObservationBiasSettings > arcwiseBiasSettings = boost::dynamic_pointer_cast<
                ArcWiseConstantObservationBiasSettings >( biasSettings );
        if( arcwiseBiasSettings == NULL )
        {
            throw std::runtime_error( "Error when making arc-wise relative observation bias, settings are inconsistent" );
        }
        else if( arcwiseBiasSettings->useAbsoluteBias_ )
        {
            throw std::runtime_error( "Error when making arc-wise relative observation bias, class contents are inconsistent" );
        }

        std::vector< Eigen::Matrix< double, ObservationSize, 1 > > observationBiases;
        for( unsigned int i = 0; i < arcwiseBiasSettings->observationBiases_.size( ); i++ )
        {
            // Check if size of bias is consistent with requested observable size
            if( arcwiseBiasSettings->observationBiases_.at( i ).rows( ) != ObservationSize )
            {
                throw std::runtime_error( "Error when making arc-wise observation bias, bias size is inconsistent" );
            }
            else
            {
                observationBiases.push_back( arcwiseBiasSettings->observationBiases_.at( i ) );
            }
        }
        observationBias = boost::make_shared< ConstantRelativeArcWiseObservationBias< ObservationSize > >(
                    arcwiseBiasSettings->arcStartTimes_, observationBiases,
                    observation_models::getLinkEndIndicesForLinkEndTypeAtObservable(
                        observableType, arcwiseBiasSettings->linkEndForTime_, linkEnds.size( ) ).at( 0 ) );
        break;
    }
    case multiple_observation_biases:
    {
        // Check input consistency
        boost::shared_ptr< MultipleObservationBiasSettings > multiBiasSettings = boost::dynamic_pointer_cast<
                MultipleObservationBiasSettings >( biasSettings );
        if( multiBiasSettings == NULL )
        {
            throw std::runtime_error( "Error when making multiple observation biases, settings are inconsistent" );
        }

        // Create list of biases
        std::vector< boost::shared_ptr< ObservationBias< ObservationSize > > > observationBiasList;
        for( unsigned int i = 0; i < multiBiasSettings->biasSettingsList_.size( ); i++ )
        {
            observationBiasList.push_back( createObservationBiasCalculator< ObservationSize >(
                                               linkEnds, observableType, multiBiasSettings->biasSettingsList_.at( i ) , bodyMap ) );
        }

        // Create combined bias object
        observationBias = boost::make_shared< MultiTypeObservationBias< ObservationSize > >(
                    observationBiasList );
        break;
    }
    default:
    {
        std::string errorMessage = "Error when making observation bias, bias type " +
                std::to_string( biasSettings->observationBiasType_  ) + " not recognized";
        throw std::runtime_error( errorMessage );
    }
    }
    return observationBias;
}

//! Interface class for creating observation models
/*!
 *  Interface class for creating observation models. This class is used instead of a single templated free function to
 *  allow ObservationModel deroved classed with different ObservationSize template arguments to be created using the same
 *  interface. This class has template specializations for each value of ObservationSize, and contains a single
 *  createObservationModel function that performs the required operation.
 */
template< int ObservationSize = 1, typename ObservationScalarType = double, typename TimeType = double >
class ObservationModelCreator
{
public:

    //! Function to create an observation model.
    /*!
     * Function to create an observation model.
     * \param linkEnds Link ends for observation model that is to be created
     * \param observationSettings Settings for observation model that is to be created.
     * \param bodyMap List of body objects that comprises the environment
     * \return Observation model of required settings.
     */
    static boost::shared_ptr< observation_models::ObservationModel<
    ObservationSize, ObservationScalarType, TimeType > > createObservationModel(
            const LinkEnds linkEnds,
            const boost::shared_ptr< ObservationSettings > observationSettings,
            const simulation_setup::NamedBodyMap &bodyMap );
};

//! Interface class for creating observation models of size 1.
template< typename ObservationScalarType, typename TimeType >
class ObservationModelCreator< 1, ObservationScalarType, TimeType >
{
public:

    //! Function to create an observation model of size 1.
    /*!
     * Function to create an observation model of size 1.
     * \param linkEnds Link ends for observation model that is to be created
     * \param observationSettings Settings for observation model that is to be created (must be for observation model if size 1).
     * \param bodyMap List of body objects that comprises the environment
     * \return Observation model of required settings.
     */
    static boost::shared_ptr< observation_models::ObservationModel<
    1, ObservationScalarType, TimeType > > createObservationModel(
            const LinkEnds linkEnds,
            const boost::shared_ptr< ObservationSettings > observationSettings,
            const simulation_setup::NamedBodyMap &bodyMap )
    {
        using namespace observation_models;

        boost::shared_ptr< observation_models::ObservationModel<
                1, ObservationScalarType, TimeType > > observationModel;

        // Check type of observation model.
        switch( observationSettings->observableType_ )
        {
        case one_way_range:
        {
            // Check consistency input.
            if( linkEnds.size( ) != 2 )
            {
                std::string errorMessage =
                        "Error when making 1 way range model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }
            if( linkEnds.count( receiver ) == 0 )
            {
                throw std::runtime_error( "Error when making 1 way range model, no receiver found" );
            }
            if( linkEnds.count( transmitter ) == 0 )
            {
                throw std::runtime_error( "Error when making 1 way range model, no transmitter found" );
            }

            boost::shared_ptr< ObservationBias< 1 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_,bodyMap );
            }

            // Create observation model
            observationModel = boost::make_shared< OneWayRangeObservationModel<
                    ObservationScalarType, TimeType > >(
                        createLightTimeCalculator< ObservationScalarType, TimeType >(
                            linkEnds.at( transmitter ), linkEnds.at( receiver ),
                            bodyMap, observationSettings->lightTimeCorrectionsList_ ),
                        observationBias );

            break;
        }
        case one_way_doppler:
        {
            // Check consistency input.
            if( linkEnds.size( ) != 2 )
            {
                std::string errorMessage =
                        "Error when making 1 way Doppler model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }
            if( linkEnds.count( receiver ) == 0 )
            {
                throw std::runtime_error( "Error when making 1 way Doppler model, no receiver found" );
            }
            if( linkEnds.count( transmitter ) == 0 )
            {
                throw std::runtime_error( "Error when making 1 way Doppler model, no transmitter found" );
            }

            boost::shared_ptr< ObservationBias< 1 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_,bodyMap );
            }

            if( boost::dynamic_pointer_cast< OneWayDopplerObservationSettings >( observationSettings ) == NULL )
            {
                // Create observation model
                observationModel = boost::make_shared< OneWayDopplerObservationModel<
                        ObservationScalarType, TimeType > >(
                            createLightTimeCalculator< ObservationScalarType, TimeType >(
                                linkEnds.at( transmitter ), linkEnds.at( receiver ),
                                bodyMap, observationSettings->lightTimeCorrectionsList_ ),
                            observationBias );
            }
            else
            {
                boost::shared_ptr< OneWayDopplerObservationSettings > oneWayDopplerSettings =
                        boost::dynamic_pointer_cast< OneWayDopplerObservationSettings >( observationSettings );
                // Create observation model
                observationModel = boost::make_shared< OneWayDopplerObservationModel<
                        ObservationScalarType, TimeType > >(
                            createLightTimeCalculator< ObservationScalarType, TimeType >(
                                linkEnds.at( transmitter ), linkEnds.at( receiver ),
                                bodyMap, observationSettings->lightTimeCorrectionsList_ ),
                            createOneWayDopplerProperTimeCalculator< ObservationScalarType, TimeType >(
                                oneWayDopplerSettings->transmitterProperTimeRateSettings_, linkEnds, bodyMap, transmitter ),
                            createOneWayDopplerProperTimeCalculator< ObservationScalarType, TimeType >(
                                oneWayDopplerSettings->receiverProperTimeRateSettings_, linkEnds, bodyMap, receiver ),
                            observationBias );
            }

            break;
        }

        case two_way_doppler:
        {
            // Check consistency input.
            if( linkEnds.size( ) != 3 )
            {
                std::string errorMessage =
                        "Error when making 2 way Doppler model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }
            if( linkEnds.count( receiver ) == 0 )
            {
                throw std::runtime_error( "Error when making 2 way Doppler model, no receiver found" );
            }

            if( linkEnds.count( reflector1 ) == 0 )
            {
                throw std::runtime_error( "Error when making 2 way Doppler model, no retransmitter found" );
            }

            if( linkEnds.count( transmitter ) == 0 )
            {
                throw std::runtime_error( "Error when making 2 way Doppler model, no transmitter found" );
            }

            boost::shared_ptr< ObservationBias< 1 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_,bodyMap );
            }

            // Create observation model

            LinkEnds uplinkLinkEnds;
            uplinkLinkEnds[ transmitter ] = linkEnds.at( transmitter );
            uplinkLinkEnds[ receiver ] = linkEnds.at( reflector1 );

            LinkEnds downlinkLinkEnds;
            downlinkLinkEnds[ transmitter ] = linkEnds.at( reflector1 );
            downlinkLinkEnds[ receiver ] = linkEnds.at( receiver );

            boost::shared_ptr< TwoWayDopplerObservationSettings > twoWayDopplerSettings =
                    boost::dynamic_pointer_cast< TwoWayDopplerObservationSettings >( observationSettings );

            if( twoWayDopplerSettings == NULL )
            {
                observationModel = boost::make_shared< TwoWayDopplerObservationModel<
                        ObservationScalarType, TimeType > >(
                            boost::dynamic_pointer_cast< OneWayDopplerObservationModel< ObservationScalarType, TimeType > >(
                                ObservationModelCreator< 1, ObservationScalarType, TimeType >::createObservationModel(
                                    uplinkLinkEnds, boost::make_shared< ObservationSettings >(
                                        one_way_doppler, observationSettings->lightTimeCorrectionsList_ ), bodyMap ) ),
                            boost::dynamic_pointer_cast< OneWayDopplerObservationModel< ObservationScalarType, TimeType > >(
                                ObservationModelCreator< 1, ObservationScalarType, TimeType >::createObservationModel(
                                    downlinkLinkEnds, boost::make_shared< ObservationSettings >(
                                        one_way_doppler, observationSettings->lightTimeCorrectionsList_ ), bodyMap ) ),
                            observationBias );
            }
            else
            {
                observationModel = boost::make_shared< TwoWayDopplerObservationModel<
                        ObservationScalarType, TimeType > >(
                            boost::dynamic_pointer_cast< OneWayDopplerObservationModel< ObservationScalarType, TimeType > >(
                                ObservationModelCreator< 1, ObservationScalarType, TimeType >::createObservationModel(
                                    uplinkLinkEnds, twoWayDopplerSettings->uplinkOneWayDopplerSettings_, bodyMap ) ),
                            boost::dynamic_pointer_cast< OneWayDopplerObservationModel< ObservationScalarType, TimeType > >(
                                ObservationModelCreator< 1, ObservationScalarType, TimeType >::createObservationModel(
                                    downlinkLinkEnds, twoWayDopplerSettings->downlinkOneWayDopplerSettings_, bodyMap ) ),
                            observationBias );
            }

            break;
        }

        case one_way_differenced_range:
        {
            boost::shared_ptr< OneWayDifferencedRangeRateObservationSettings > rangeRateObservationSettings =
                    boost::dynamic_pointer_cast< OneWayDifferencedRangeRateObservationSettings >( observationSettings );
            if( rangeRateObservationSettings == NULL )
            {
                throw std::runtime_error( "Error when making differenced one-way range rate, input type is inconsistent" );
            }
            // Check consistency input.
            if( linkEnds.size( ) != 2 )
            {
                std::string errorMessage =
                        "Error when making 1 way range model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }
            if( linkEnds.count( receiver ) == 0 )
            {
                throw std::runtime_error( "Error when making 1 way range model, no receiver found" );
            }
            if( linkEnds.count( transmitter ) == 0 )
            {
                throw std::runtime_error( "Error when making 1 way range model, no transmitter found" );
            }

            boost::shared_ptr< ObservationBias< 1 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_,bodyMap );
            }

            // Create observation model
            observationModel = boost::make_shared< OneWayDifferencedRangeObservationModel<
                    ObservationScalarType, TimeType > >(
                        createLightTimeCalculator< ObservationScalarType, TimeType >(
                            linkEnds.at( transmitter ), linkEnds.at( receiver ),
                            bodyMap, observationSettings->lightTimeCorrectionsList_ ),
                        createLightTimeCalculator< ObservationScalarType, TimeType >(
                            linkEnds.at( transmitter ), linkEnds.at( receiver ),
                            bodyMap, observationSettings->lightTimeCorrectionsList_ ),
                        rangeRateObservationSettings->integrationTimeFunction_,
                        observationBias );

            break;
        }
        case n_way_range:
        {
            // Check consistency input.
            if( linkEnds.size( ) < 2 )
            {
                std::string errorMessage =
                        "Error when making n way range model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }
            if( linkEnds.count( receiver ) == 0 )
            {
                throw std::runtime_error( "Error when making n way range model, no receiver found" );
            }

            if( linkEnds.count( transmitter ) == 0 )
            {
                throw std::runtime_error( "Error when making n way range model, no transmitter found" );
            }

            // Check link end consistency.
            for( LinkEnds::const_iterator linkEndIterator = linkEnds.begin( ); linkEndIterator != linkEnds.end( );
                 linkEndIterator++ )
            {
                if( ( linkEndIterator->first != transmitter ) && ( linkEndIterator->first != receiver ) )
                {
                    int linkEndIndex = static_cast< int >( linkEndIterator->first );
                    LinkEndType previousLinkEndType = static_cast< LinkEndType >( linkEndIndex - 1 );

                    if( linkEnds.count( previousLinkEndType ) == 0 )
                    {
                        throw std::runtime_error( "Error when making n-way range model, did not find link end type " +
                                                  std::to_string( previousLinkEndType ) );
                    }
                }
            }

            // Create observation bias object
            boost::shared_ptr< ObservationBias< 1 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_, bodyMap );
            }

            std::vector< boost::shared_ptr< LightTimeCorrectionSettings > > lightTimeCorrectionsList;

            boost::function< std::vector< double >( const double ) > retransmissionTimesFunction_;

            boost::shared_ptr< NWayRangeObservationSettings > nWayRangeObservationSettings =
                    boost::dynamic_pointer_cast< NWayRangeObservationSettings >( observationSettings );

            if( nWayRangeObservationSettings == NULL )
            {
                lightTimeCorrectionsList = observationSettings->lightTimeCorrectionsList_;
            }
            else if( nWayRangeObservationSettings->oneWayRangeObsevationSettings_.size( ) != linkEnds.size( ) - 1 )
            {
                throw std::runtime_error( "Error whaen making n-way range, input data is inconsistent" );
            }
            else
            {
                retransmissionTimesFunction_ = nWayRangeObservationSettings->retransmissionTimesFunction_;
            }

            // Define light-time calculator list
            std::vector< boost::shared_ptr< LightTimeCalculator< ObservationScalarType, TimeType > > > lightTimeCalculators;

            // Iterate over all link ends and create light-time calculators
            LinkEnds::const_iterator transmitterIterator = linkEnds.begin( );
            LinkEnds::const_iterator receiverIterator = linkEnds.begin( );
            receiverIterator++;
            for( unsigned int i = 0; i < linkEnds.size( ) - 1; i++ )
            {
                if( nWayRangeObservationSettings != NULL )
                {
                    if( nWayRangeObservationSettings->oneWayRangeObsevationSettings_.at( i )->observableType_ != one_way_range )
                    {
                        throw std::runtime_error( "Error in n-way observable creation, consituent link is not of type 1-way" );
                    }
                    lightTimeCalculators.push_back(
                                createLightTimeCalculator< ObservationScalarType, TimeType >(
                                    transmitterIterator->second, receiverIterator->second,
                                    bodyMap, nWayRangeObservationSettings->oneWayRangeObsevationSettings_.at( i )->
                                    lightTimeCorrectionsList_ ) );
                }
                else
                {
                    lightTimeCalculators.push_back(
                                createLightTimeCalculator< ObservationScalarType, TimeType >(
                                    transmitterIterator->second, receiverIterator->second,
                                    bodyMap, observationSettings->lightTimeCorrectionsList_ ) );
                }

                transmitterIterator++;
                receiverIterator++;
            }

            // Create observation model
            observationModel = boost::make_shared< NWayRangeObservationModel<
                    ObservationScalarType, TimeType > >(
                        lightTimeCalculators, retransmissionTimesFunction_,
                        observationBias );
            break;
        }

        default:
            std::string errorMessage = "Error, observable " + std::to_string(
                        observationSettings->observableType_ ) +
                    "  not recognized when making size 1 observation model.";
            throw std::runtime_error( errorMessage );
        }
        return observationModel;
    }

};

//! Interface class for creating observation models of size 2.
template< typename ObservationScalarType, typename TimeType >
class ObservationModelCreator< 2, ObservationScalarType, TimeType >
{
public:

    //! Function to create an observation model of size 2.
    /*!
     * Function to create an observation model of size 2.
     * \param linkEnds Link ends for observation model that is to be created
     * \param observationSettings Settings for observation model that is to be created (must be for observation model if size 1).
     * \param bodyMap List of body objects that comprises the environment
     * \return Observation model of required settings.
     */
    static boost::shared_ptr< observation_models::ObservationModel<
    2, ObservationScalarType, TimeType > > createObservationModel(
            const LinkEnds linkEnds,
            const boost::shared_ptr< ObservationSettings > observationSettings,
            const simulation_setup::NamedBodyMap &bodyMap )
    {
        using namespace observation_models;
        boost::shared_ptr< observation_models::ObservationModel<
                2, ObservationScalarType, TimeType > > observationModel;

        // Check type of observation model.
        switch( observationSettings->observableType_ )
        {
        case angular_position:
        {
            // Check consistency input.
            if( linkEnds.size( ) != 2 )
            {
                std::string errorMessage =
                        "Error when making angular position model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }
            if( linkEnds.count( receiver ) == 0 )
            {
                throw std::runtime_error( "Error when making angular position model, no receiver found" );
            }
            if( linkEnds.count( transmitter ) == 0 )
            {
                throw std::runtime_error( "Error when making angular position model, no transmitter found" );
            }


            boost::shared_ptr< ObservationBias< 2 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator< 2 >(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_,bodyMap );
            }

            // Create observation model
            observationModel = boost::make_shared< AngularPositionObservationModel<
                    ObservationScalarType, TimeType > >(
                        createLightTimeCalculator< ObservationScalarType, TimeType >(
                            linkEnds.at( transmitter ), linkEnds.at( receiver ),
                            bodyMap, observationSettings->lightTimeCorrectionsList_ ),
                        observationBias );

            break;
        }
        default:
            std::string errorMessage = "Error, observable " + std::to_string(
                        observationSettings->observableType_ ) +
                    "  not recognized when making size 2 observation model.";
            throw std::runtime_error( errorMessage );
            break;
        }

        return observationModel;
    }

};

//! Interface class for creating observation models of size 3.
template< typename ObservationScalarType, typename TimeType >
class ObservationModelCreator< 3, ObservationScalarType, TimeType >
{
public:

    //! Function to create an observation model of size 3.
    /*!
     * Function to create an observation model of size 3.
     * \param linkEnds Link ends for observation model that is to be created
     * \param observationSettings Settings for observation model that is to be created (must be for observation model if size 1).
     * \param bodyMap List of body objects that comprises the environment
     * \return Observation model of required settings.
     */
    static boost::shared_ptr< observation_models::ObservationModel<
    3, ObservationScalarType, TimeType > > createObservationModel(
            const LinkEnds linkEnds,
            const boost::shared_ptr< ObservationSettings > observationSettings,
            const simulation_setup::NamedBodyMap &bodyMap )
    {
        using namespace observation_models;
        boost::shared_ptr< observation_models::ObservationModel<
                3, ObservationScalarType, TimeType > > observationModel;

        // Check type of observation model.
        switch( observationSettings->observableType_ )
        {
        case position_observable:
        {
            // Check consistency input.
            if( linkEnds.size( ) != 1 )
            {
                std::string errorMessage =
                        "Error when making position observable model, " +
                        std::to_string( linkEnds.size( ) ) + " link ends found";
                throw std::runtime_error( errorMessage );
            }

            if( linkEnds.count( observed_body ) == 0 )
            {
                throw std::runtime_error( "Error when making position observable model, no observed_body found" );
            }

            if( observationSettings->lightTimeCorrectionsList_.size( ) > 0 )
            {
                throw std::runtime_error( "Error when making position observable model, found light time corrections" );
            }
            if( linkEnds.at( observed_body ).second != "" )
            {
                throw std::runtime_error( "Error, cannot yet create position function for reference point" );
            }

            boost::shared_ptr< ObservationBias< 3 > > observationBias;
            if( observationSettings->biasSettings_ != NULL )
            {
                observationBias =
                        createObservationBiasCalculator< 3 >(
                            linkEnds, observationSettings->observableType_, observationSettings->biasSettings_,bodyMap );
            }


            // Create observation model
            observationModel = boost::make_shared< PositionObservationModel<
                    ObservationScalarType, TimeType > >(
                        boost::bind( &simulation_setup::Body::getStateInBaseFrameFromEphemeris<
                                     ObservationScalarType, TimeType >,
                                     bodyMap.at( linkEnds.at( observed_body ).first ), _1 ),
                        observationBias );

            break;
        }
        default:
            std::string errorMessage = "Error, observable " + std::to_string(
                        observationSettings->observableType_ ) +
                    "  not recognized when making size 3 observation model.";
            throw std::runtime_error( errorMessage );
            break;
        }
        return observationModel;
    }
};

//! Function to create an object to simulate observations of a given type
/*!
 *  Function to create an object to simulate observations of a given type
 *  \param observableType Type of observable for which object is to simulate ObservationSimulator
 *  \param settingsPerLinkEnds Map of settings for the observation models that are to be created in the simulator object: one
 *  for each required set of link ends (each settings object must be consistent with observableType).
 *  \param bodyMap Map of Body objects that comprise the environment
 *  \return Object that simulates the observables according to the provided settings.
 */
template< int ObservationSize = 1, typename ObservationScalarType = double, typename TimeType = double >
boost::shared_ptr< ObservationSimulator< ObservationSize, ObservationScalarType, TimeType > > createObservationSimulator(
        const ObservableType observableType,
        const std::map< LinkEnds, boost::shared_ptr< ObservationSettings  > > settingsPerLinkEnds,
        const simulation_setup::NamedBodyMap &bodyMap )
{
    std::map< LinkEnds, boost::shared_ptr< ObservationModel< ObservationSize, ObservationScalarType, TimeType > > >
            observationModels;

    // Iterate over all link ends
    for( std::map< LinkEnds, boost::shared_ptr< ObservationSettings  > >::const_iterator settingIterator =
         settingsPerLinkEnds.begin( ); settingIterator != settingsPerLinkEnds.end( ); settingIterator++ )
    {
        observationModels[ settingIterator->first ] = ObservationModelCreator<
                ObservationSize, ObservationScalarType, TimeType >::createObservationModel(
                    settingIterator->first, settingIterator->second, bodyMap );
    }

    return boost::make_shared< ObservationSimulator< ObservationSize, ObservationScalarType, TimeType > >(
                observableType, observationModels );
}

//! Function to create a map of object to simulate observations (one object for each type of observable).
/*!
 *  Function to create a map of object to simulate observations (one object for each type of observable).
 *  \param observationSettingsMap Map of settings for the observation models that are to be created in the simulator object: first
 *  map key is observable type, second is link ends for observation. One observation settings object must be given
 *  for each required set of link ends/observable (each settings object must be consistent with observable type in first entry).
 *  \param bodyMap Map of Body objects that comprise the environment
 *  \return List of objects that simulate the observables according to the provided settings.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType,
boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > > createObservationSimulators(
        observation_models::SortedObservationSettingsMap observationSettingsMap,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    std::map< ObservableType,
            boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > > observationSimulators;

    // Iterate over all observables
    typedef std::map< ObservableType, std::map< LinkEnds, boost::shared_ptr< ObservationSettings > > >
            SortedObservationSettingsMap;
    for( SortedObservationSettingsMap::const_iterator settingsIterator = observationSettingsMap.begin( );
         settingsIterator != observationSettingsMap.end( ); settingsIterator++ )
    {
        // Call createObservationSimulator of required observation size
        int observableSize = getObservableSize( settingsIterator->first );
        switch( observableSize )
        {
        case 1:
        {
            observationSimulators[ settingsIterator->first ] = createObservationSimulator< 1, ObservationScalarType, TimeType >(
                        settingsIterator->first, settingsIterator->second, bodyMap );
            break;
        }
        case 2:
        {
            observationSimulators[ settingsIterator->first ] = createObservationSimulator< 2, ObservationScalarType, TimeType >(
                        settingsIterator->first, settingsIterator->second, bodyMap );
            break;
        }
        case 3:
        {
            observationSimulators[ settingsIterator->first ] = createObservationSimulator< 3, ObservationScalarType, TimeType >(
                        settingsIterator->first, settingsIterator->second, bodyMap );
            break;
        }
        default:
            throw std::runtime_error( "Error, cannot create observation simulator for size other than 1,2 and 3 ");
        }
    }
    return observationSimulators;
}


//! Function to create a map of object to simulate observations (one object for each type of observable).
/*!
 *  Function to create a map of object to simulate observations (one object for each type of observable), from a list of
 *  observation settings not sorted by observable type.
 *  \param observationSettingsMap Multi-map of settings for the observation models that are to be created in the simulator object
 *  map key is link ends for observation.
 *  \param bodyMap Map of Body objects that comprise the environment
 *  \return List of objects that simulate the observables according to the provided settings.
 */
template< typename ObservationScalarType = double, typename TimeType = double >
std::map< ObservableType,
boost::shared_ptr< ObservationSimulatorBase< ObservationScalarType, TimeType > > > createObservationSimulators(
        observation_models::ObservationSettingsMap observationSettingsMap,
        const simulation_setup::NamedBodyMap &bodyMap )
{
    return createObservationSimulators< ObservationScalarType, TimeType >(
                convertUnsortedToSortedObservationSettingsMap( observationSettingsMap ), bodyMap );
}

//! Function to filter list of observationViabilitySettings, so that only those relevant for single set of link ends are retained
/*!
 * Function to filter list of observationViabilitySettings, so that only those relevant for single set of link ends are retained
 * \param observationViabilitySettings Full, unfiltered, list of observation viability settings
 * \param linkEnds Link ends for which the relevant observation vaibilityies are to be retrieved
 * \return List of observationViabilitySettings that are relevant for linkEnds
 */
ObservationViabilitySettingsList filterObservationViabilitySettings(
        const ObservationViabilitySettingsList& observationViabilitySettings,
        const LinkEnds& linkEnds );

//! Function to retrieve the link end indices in link end states/times that are to be used in viability calculation
/*!
 * Function to retrieve the link end indices in link end states/times that are to be used in viability calculation.
 * Return variable is a vector of pairs, where each the first entry denotes the index of the point at which the link is to be
 * checkd. The second entry denotes the index for the opposite end of teh link.
 * \param linkEnds Complete set of link ends for which check is to be performed
 * \param observableType Observable type for which check is to be performed
 * \param linkEndToCheck Link end at which check is to be performed
 * \return Link end indices in link end states/times that are to be used in viability calculation
 */
std::vector< std::pair< int, int > > getLinkEndIndicesForObservationViability(
        const LinkEnds& linkEnds,
        const ObservableType observableType,
        const LinkEndId linkEndToCheck );

//! Function to create an object to check if a minimum elevation angle condition is met for an observation
/*!
 * Function to create an object to check if a minimum elevation angle condition is met for an observation
 * \param bodyMap Map of body objects that constitutes the environment
 * \param linkEnds Link ends for which viability check object is to be made
 * \param observationType Type of observable for which viability check object is to be made
 * \param observationViabilitySettings Object that defines the settings for the creation of the viability check creation
 * (settings must be compatible with minimum elevation angle check).  Ground station must ve specified by
 * associatedLinkEnd_.second in observationViabilitySettings.
 * \param stationName Name of the ground station for which calculator is to be computed (if no station is explicitly given in
 * observationViabilitySettings).
 * \return Object to check if a minimum elevation angle condition is met for an observation
 */
boost::shared_ptr< MinimumElevationAngleCalculator > createMinimumElevationAngleCalculator(
        const simulation_setup::NamedBodyMap& bodyMap,
        const LinkEnds linkEnds,
        const ObservableType observationType,
        const boost::shared_ptr< ObservationViabilitySettings > observationViabilitySettings,
        const std::string& stationName );

//! Function to create an object to check if a body avoidance angle condition is met for an observation
/*!
 * Function to create an object to check if a body avoidance angle condition is met for an observation
 * \param bodyMap Map of body objects that constitutes the environment
 * \param linkEnds Link ends for which viability check object is to be made
 * \param observationType Type of observable for which viability check object is to be made
 * \param observationViabilitySettings Object that defines the settings for the creation of the viability check creation
 * (settings must be compatible with body avoidance angle check). If ground station is not specified (by
 * associatedLinkEnd_.second in observationViabilitySettings), check is performed for all ground stations on (or c.o.m. of) body
 * (defined by associatedLinkEnd_.first) automatically.
 * \return Object to check if a body avoidance angle condition is met for an observation
 */
boost::shared_ptr< BodyAvoidanceAngleCalculator > createBodyAvoidanceAngleCalculator(
        const simulation_setup::NamedBodyMap& bodyMap,
        const LinkEnds linkEnds,
        const ObservableType observationType,
        const boost::shared_ptr< ObservationViabilitySettings > observationViabilitySettings );

//! Function to create an object to check if a body occultation condition is met for an observation
/*!
 * Function to create an object to check if a body occultation condition is met for an observation
 * \param bodyMap Map of body objects that constitutes the environment
 * \param linkEnds Link ends for which viability check object is to be made
 * \param observationType Type of observable for which viability check object is to be made
 * \param observationViabilitySettings Object that defines the settings for the creation of the viability check creation
 * (settings must be compatible with body occultation check).  If ground station is not specified (by
 * associatedLinkEnd_.second in observationViabilitySettings), check is performed for all ground stations on (or c.o.m. of) body
 * (defined by associatedLinkEnd_.first) automatically, or fo
 * \return Object to check if a body occultation condition is met for an observation
 */
boost::shared_ptr< OccultationCalculator > createOccultationCalculator(
        const simulation_setup::NamedBodyMap& bodyMap,
        const LinkEnds linkEnds,
        const ObservableType observationType,
        const boost::shared_ptr< ObservationViabilitySettings > observationViabilitySettings );

//! Function to create an list of obervation viability conditions for a single set of link ends
/*!
 * Function to create an list of obervation viability conditions for a single set of link ends
 * \param bodyMap Map of body objects that constitutes the environment
 * \param linkEnds Link ends for which viability check object is to be made
 * \param observationType Type of observable for which viability check object is to be made
 * \param observationViabilitySettings List of viability settings from which viability check objects are to be created
 * \return List of obervation viability conditions for a single set of link ends
 */
std::vector< boost::shared_ptr< ObservationViabilityCalculator > > createObservationViabilityCalculators(
        const simulation_setup::NamedBodyMap& bodyMap,
        const LinkEnds linkEnds,
        const ObservableType observationType,
        const std::vector< boost::shared_ptr< ObservationViabilitySettings > >& observationViabilitySettings );

//! Function to create an list of obervation viability conditions for a number of sets of link ends, for a single observable type
/*!
 * Function to create an list of obervation viability conditions for a number of sets of link ends, for a single observable type
 * \param bodyMap Map of body objects that constitutes the environment
 * \param linkEnds List of link ends for which viability check object is to be made
 * \param observationType Type of observable for which viability check object is to be made
 * \param observationViabilitySettings List of viability settings from which viability check objects are to be created
 * \return List of obervation viability conditions for a number of sets of link ends, for a single observable type
 */
std::map< LinkEnds, std::vector< boost::shared_ptr< ObservationViabilityCalculator > > > createObservationViabilityCalculators(
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::vector< LinkEnds > linkEnds,
        const ObservableType observationType,
        const std::vector< boost::shared_ptr< ObservationViabilitySettings > >& observationViabilitySettings );

//! Function to create a list of obervation viability conditions for any number of sets of link ends and observable types
/*!
 * Function to create a list of obervation viability conditions for any number of sets of link ends and observable types
 * \param bodyMap Map of body objects that constitutes the environment
 * \param linkEndsPerObservable List of link ends, for each observable type, for which viability check object is to be made
 * \param observationViabilitySettings List of viability settings from which viability check objects are to be created
 * \return List of obervation viability conditions for any number of sets of link ends and observable types
 */
PerObservableObservationViabilityCalculatorList
createObservationViabilityCalculators(
        const simulation_setup::NamedBodyMap& bodyMap,
        const std::map< ObservableType, std::vector< LinkEnds > > linkEndsPerObservable,
        const std::vector< boost::shared_ptr< ObservationViabilitySettings > >& observationViabilitySettings );

} // namespace observation_models

} // namespace tudat

#endif // TUDAT_CREATEOBSERVATIONMODEL_H
