/*    Copyright (c) 2010-2018, Delft University of Technology
 *    All rigths reserved
 *
 *    This file is part of the Tudat. Redistribution and use in source and
 *    binary forms, with or without modification, are permitted exclusively
 *    under the terms of the Modified BSD license. You should have received
 *    a copy of the license with this file. If not, please or visit:
 *    http://tudat.tudelft.nl/LICENSE.
 */

#include "Tudat/Astrodynamics/BasicAstrodynamics/accelerationModelTypes.h"
#include "Tudat/Astrodynamics/BasicAstrodynamics/torqueModelTypes.h"
#include "Tudat/SimulationSetup/PropagationSetup/createEnvironmentUpdater.h"

namespace tudat
{

namespace propagators
{

//! Function to check whether the requested environment updates are
//! possible with the existing environment.
void checkValidityOfRequiredEnvironmentUpdates(
        const std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >&
        requestedUpdates, const simulation_setup::NamedBodyMap& bodyMap )
{
    using namespace propagators;

    // Iterate over all environment update types.
    for( std::map< propagators::EnvironmentModelsToUpdate,
         std::vector< std::string > >::const_iterator
         updateIterator = requestedUpdates.begin( );
         updateIterator != requestedUpdates.end( ); updateIterator++ )
    {
        // Iterate over all updated bodies.
        for( unsigned int i = 0; i < updateIterator->second.size( ); i++ )
        {
            // Ignore global required updates.
            if( updateIterator->second.at( i ) != "" )
            {
                // Check if body exists.
                if( bodyMap.count( updateIterator->second.at( i ) ) == 0 )
                {
                    throw std::runtime_error(
                                "Error when making environment model update settings, could not find body "
                                + updateIterator->second.at( i ) );
                }

                // Check if requested environment model exists.
                switch( updateIterator->first )
                {
                case body_transational_state_update:
                {
                    if( bodyMap.at( updateIterator->second.at( i ) )->getEphemeris( ) == NULL )
                    {
                        throw std::runtime_error(
                                    "Error when making environment model update settings, could not find ephemeris of body "
                                    + updateIterator->second.at( i ) );
                    }
                    break;
                }
                case body_rotational_state_update:
                {
                    if( ( bodyMap.at( updateIterator->second.at( i ) )->
                          getRotationalEphemeris( ) == NULL ) &&
                            ( bodyMap.at( updateIterator->second.at( i ) )->getDependentOrientationCalculator( ) == NULL ) )
                    {
                        throw std::runtime_error(
                                    "Error when making environment model update settings, could not find rotational ephemeris or dependent orientation calculator of body "
                                    + updateIterator->second.at( i ) );
                    }
                    break;
                }
                case spherical_harmonic_gravity_field_update:
                {
                    boost::shared_ptr< gravitation::SphericalHarmonicsGravityField >
                            gravityFieldModel =
                            boost::dynamic_pointer_cast< gravitation::SphericalHarmonicsGravityField >(
                                bodyMap.at( updateIterator->second.at( i ) )->getGravityFieldModel( ) );
                    if( gravityFieldModel == NULL )
                    {
                        throw std::runtime_error(
                                    "Error when making environment model update settings, could not find spherical harmonic gravity field of body "
                                    + updateIterator->second.at( i ) );
                    }
                    break;
                }
                case vehicle_flight_conditions_update:
                {
                    boost::shared_ptr< aerodynamics::FlightConditions > flightConditions = bodyMap.at(
                                updateIterator->second.at( i ) )->getFlightConditions( );
                    if( flightConditions == NULL )
                    {
                        throw std::runtime_error(
                                    "Error when making environment model update settings, could not find flight conditions of body "
                                    + updateIterator->second.at( i ) );
                    }
                    break;
                }
                case radiation_pressure_interface_update:
                {
                    std::map< std::string, boost::shared_ptr< electro_magnetism::RadiationPressureInterface > >
                            radiationPressureInterfaces = bodyMap.at(
                                updateIterator->second.at( i ) )->getRadiationPressureInterfaces( );
                    if( radiationPressureInterfaces.size( ) == 0 )
                    {
                        throw std::runtime_error(
                                    "Error when making environment model update settings, could not find radiation pressure interface of body "
                                    + updateIterator->second.at( i ) );
                    }
                    break;
                }
                case body_mass_update:
                    if( bodyMap.at( updateIterator->second.at( i ) )->getBodyMassFunction( ) == NULL )
                    {
                        throw std::runtime_error(
                                    "Error when making environment model update settings, no body mass function of body "
                                    + updateIterator->second.at( i ) );
                    }

                    break;
                }
            }
        }
    }
}

//! Function that removes propagated states from the updated environment variables
void removePropagatedStatesFomEnvironmentUpdates(
        std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >& environmentModelsToUpdate,
        const std::map< IntegratedStateType, std::vector< std::pair< std::string, std::string > > >& integratedStateList )
{
    // Iterate over all propagated states
    for( auto it = integratedStateList.begin( ); it != integratedStateList.end( ); it++ )
    {
        // Iterate over all propagated bodies for current state
        for( unsigned int i = 0; i < it->second.size( ); i++ )
        {
            switch( it->first )
            {
            // Check for propagated translational states in update list, and remove if necessary
            case translational_state:
                if( environmentModelsToUpdate.count( body_translational_state_update ) > 0 )
                {
                    std::vector< std::string > bodiesToUpdate = environmentModelsToUpdate.at( body_translational_state_update );
                    std::vector< std::string >::iterator findIterator =
                            std::find( bodiesToUpdate.begin( ), bodiesToUpdate.end( ), it->second.at( i ).first );

                    if( findIterator != bodiesToUpdate.end( ) )
                    {
                        bodiesToUpdate.erase( findIterator );
                        environmentModelsToUpdate[ body_translational_state_update ] = bodiesToUpdate;

                    }
                }
                break;
            // Check for propagated rotational states in update list, and remove if necessary
            case rotational_state:
                if( environmentModelsToUpdate.count( body_rotational_state_update ) > 0 )
                {
                    std::vector< std::string > bodiesToUpdate = environmentModelsToUpdate.at( body_rotational_state_update );
                    std::vector< std::string >::iterator findIterator =
                            std::find( bodiesToUpdate.begin( ), bodiesToUpdate.end( ), it->second.at( i ).first );

                    if( findIterator != bodiesToUpdate.end( ) )
                    {
                        bodiesToUpdate.erase( findIterator );
                        environmentModelsToUpdate[ body_rotational_state_update ] = bodiesToUpdate;

                    }
                }
                break;
            // Check for propagated mass states in update list, and remove if necessary
            case body_mass_state:
                if( environmentModelsToUpdate.count( body_mass_update ) > 0 )
                {
                    std::vector< std::string > bodiesToUpdate = environmentModelsToUpdate.at( body_mass_update );
                    std::vector< std::string >::iterator findIterator =
                            std::find( bodiesToUpdate.begin( ), bodiesToUpdate.end( ), it->second.at( i ).first );

                    if( findIterator != bodiesToUpdate.end( ) )
                    {
                        bodiesToUpdate.erase( findIterator );
                        environmentModelsToUpdate[ body_mass_update ] = bodiesToUpdate;

                    }
                }
                break;
            case custom_state:
                break;
            default:
                throw std::runtime_error( "Error when removing propagated states from environment updates, state type " +
                                          std::to_string( it->first ) + " not recognized." );
            }
        }
    }
}

//! Get list of required environment model update settings from torque models.
std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >
createRotationalEquationsOfMotionEnvironmentUpdaterSettings(
        const basic_astrodynamics::TorqueModelMap& torqueModels, const simulation_setup::NamedBodyMap& bodyMap )
{
    using namespace basic_astrodynamics;

    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > environmentModelsToUpdate;
    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > singleTorqueUpdateNeeds;

    // Iterate over all bodies on which torques are being exerting
    for( TorqueModelMap::const_iterator acceleratedBodyIterator = torqueModels.begin( );
         acceleratedBodyIterator != torqueModels.end( ); acceleratedBodyIterator++ )
    {
        // Iterate over all bodies exerting on current body
        for( SingleBodyTorqueModelMap::const_iterator torqueModelIterator = acceleratedBodyIterator->second.begin( );
             torqueModelIterator != acceleratedBodyIterator->second.end( ); torqueModelIterator++ )
        {
            singleTorqueUpdateNeeds.clear( );
            for( unsigned int i = 0; i < torqueModelIterator->second.size( ); i++ )
            {
                AvailableTorque currentTorqueModelType =
                        getTorqueModelType( torqueModelIterator->second.at( i ) );

                switch( currentTorqueModelType )
                {
                case second_order_gravitational_torque:
                    break;
                case aerodynamic_torque:
                    singleTorqueUpdateNeeds[ body_rotational_state_update ].push_back(
                                torqueModelIterator->first );
                    singleTorqueUpdateNeeds[ vehicle_flight_conditions_update ].push_back(
                                acceleratedBodyIterator->first );
                    break;
                default:
                    std::cerr << "Error, update information not found for torque model " << currentTorqueModelType << std::endl;
                    break;
                }
            }

            checkValidityOfRequiredEnvironmentUpdates( singleTorqueUpdateNeeds, bodyMap );
            addEnvironmentUpdates( environmentModelsToUpdate, singleTorqueUpdateNeeds );
        }
    }

    return environmentModelsToUpdate;
}


//! Get list of required environment model update settings from translational acceleration models.
std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >
createTranslationalEquationsOfMotionEnvironmentUpdaterSettings(
        const basic_astrodynamics::AccelerationMap& translationalAccelerationModels,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    using namespace basic_astrodynamics;
    using namespace propagators;

    std::map< propagators::EnvironmentModelsToUpdate,
            std::vector< std::string > > environmentModelsToUpdate;
    std::map< propagators::EnvironmentModelsToUpdate,
            std::vector< std::string > > singleAccelerationUpdateNeeds;

    // Iterate over all bodies being accelerated
    for( AccelerationMap::const_iterator acceleratedBodyIterator
         = translationalAccelerationModels.begin( );
         acceleratedBodyIterator != translationalAccelerationModels.end( );
         acceleratedBodyIterator++ )
    {
        // Iterate over all bodies exerting acceleration.
        for( SingleBodyAccelerationMap::const_iterator accelerationModelIterator
             = acceleratedBodyIterator->second.begin( );
             accelerationModelIterator != acceleratedBodyIterator->second.end( );
             accelerationModelIterator++ )
        {
            singleAccelerationUpdateNeeds.clear( );
            for( unsigned int i = 0; i < accelerationModelIterator->second.size( ); i++ )
            {
                AvailableAcceleration currentAccelerationModelType =
                        getAccelerationModelType( accelerationModelIterator->second.at( i ) );

                // Add translational state of both bodies to update list for current acceleration model.
                if( translationalAccelerationModels.count( accelerationModelIterator->first ) == 0 )
                {
                    singleAccelerationUpdateNeeds[ body_transational_state_update ].push_back(
                                accelerationModelIterator->first );
                }

                // Check acceleration model type and change environment update list accordingly.
                switch( currentAccelerationModelType )
                {
                case central_gravity:
                    break;
                case third_body_central_gravity:
                {
                    boost::shared_ptr< gravitation::ThirdBodyCentralGravityAcceleration >
                            thirdBodyAcceleration = boost::dynamic_pointer_cast<
                            gravitation::ThirdBodyCentralGravityAcceleration >(
                                accelerationModelIterator->second.at( i ) );
                    if( thirdBodyAcceleration != NULL && translationalAccelerationModels.count(
                                thirdBodyAcceleration->getCentralBodyName( ) ) == 0 )
                    {
                        if( translationalAccelerationModels.count( thirdBodyAcceleration->getCentralBodyName( ) ) == 0 )
                        {
                            singleAccelerationUpdateNeeds[ body_transational_state_update ].push_back(
                                        thirdBodyAcceleration->getCentralBodyName( ) );
                        }
                    }
                    else if( thirdBodyAcceleration == NULL )
                    {
                        throw std::runtime_error(
                                    std::string( "Error, incompatible input (ThirdBodyCentralGravityAcceleration) to" )
                                    + std::string(  "createTranslationalEquationsOfMotionEnvironmentUpdaterSettings" ) );
                    }
                    break;
                }
                case aerodynamic:
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ vehicle_flight_conditions_update ].push_back(
                                acceleratedBodyIterator->first );
                    singleAccelerationUpdateNeeds[ body_mass_update ].push_back(
                                acceleratedBodyIterator->first );
                    break;
                case cannon_ball_radiation_pressure:
                    singleAccelerationUpdateNeeds[ radiation_pressure_interface_update ].push_back(
                                acceleratedBodyIterator->first );
                    singleAccelerationUpdateNeeds[ body_mass_update ].push_back(
                                acceleratedBodyIterator->first );
                    break;
                case spherical_harmonic_gravity:
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].
                            push_back( accelerationModelIterator->first );
                    break;
                case mutual_spherical_harmonic_gravity:
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                acceleratedBodyIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].push_back(
                                acceleratedBodyIterator->first );
                    break;
                case third_body_spherical_harmonic_gravity:
                {
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].
                            push_back( accelerationModelIterator->first );

                    boost::shared_ptr<
                            gravitation::ThirdBodySphericalHarmonicsGravitationalAccelerationModel >
                            thirdBodyAcceleration = boost::dynamic_pointer_cast<
                            gravitation::ThirdBodySphericalHarmonicsGravitationalAccelerationModel >(
                                accelerationModelIterator->second.at( i ) );;
                    if( thirdBodyAcceleration != NULL && translationalAccelerationModels.count(
                                thirdBodyAcceleration->getCentralBodyName( ) ) == 0  )
                    {
                        singleAccelerationUpdateNeeds[ body_transational_state_update ].push_back(
                                    thirdBodyAcceleration->getCentralBodyName( ) );
                    }
                    else if( thirdBodyAcceleration == NULL )
                    {
                        throw std::runtime_error(
                                    std::string( "Error, incompatible input (ThirdBodySphericalHarmonicsGravitational" )
                                    + std::string( "AccelerationModel) to createTranslationalEquationsOfMotion ")
                                    + std::string( "EnvironmentUpdaterSettings" ) );
                    }
                    break;
                }
                case third_body_mutual_spherical_harmonic_gravity:
                {
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                acceleratedBodyIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].push_back(
                                acceleratedBodyIterator->first );

                    boost::shared_ptr< gravitation::ThirdBodyMutualSphericalHarmonicsGravitationalAccelerationModel >
                            thirdBodyAcceleration = boost::dynamic_pointer_cast<
                            gravitation::ThirdBodyMutualSphericalHarmonicsGravitationalAccelerationModel >(
                                accelerationModelIterator->second.at( i ) );
                    if( thirdBodyAcceleration != NULL && translationalAccelerationModels.count(
                                thirdBodyAcceleration->getCentralBodyName( ) ) == 0  )
                    {
                        singleAccelerationUpdateNeeds[ body_transational_state_update ].push_back(
                                    thirdBodyAcceleration->getCentralBodyName( ) );
                        singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                    thirdBodyAcceleration->getCentralBodyName( ) );
                        singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].push_back(
                                    thirdBodyAcceleration->getCentralBodyName( ) );
                    }
                    else if( thirdBodyAcceleration == NULL )
                    {
                        throw std::runtime_error(
                                    std::string( "Error, incompatible input (ThirdBodyMutualSphericalHarmonicsGravitational" ) +
                                    std::string( "AccelerationModel) to createTranslationalEquationsOfMotion ") +
                                    std::string( "EnvironmentUpdaterSettings" ) );
                    }
                    break;
                }
                case thrust_acceleration:
                {
                    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > thrustModelUpdates =
                            boost::dynamic_pointer_cast< propulsion::ThrustAcceleration >(
                                accelerationModelIterator->second.at( i ) )->getRequiredModelUpdates( );
                    addEnvironmentUpdates( singleAccelerationUpdateNeeds, thrustModelUpdates );

                    singleAccelerationUpdateNeeds[ body_mass_update ].push_back(
                                acceleratedBodyIterator->first );

                    break;
                }
                case relativistic_correction_acceleration:
                {
                    boost::shared_ptr< relativity::RelativisticAccelerationCorrection >
                            accelerationCorrection = boost::dynamic_pointer_cast< relativity::RelativisticAccelerationCorrection >(
                                accelerationModelIterator->second.at( i ) );
                    if( accelerationCorrection->getCalculateDeSitterCorrection( ) )
                    {
                        std::string primaryBody = accelerationCorrection->getPrimaryBodyName( );
                        if( translationalAccelerationModels.count(primaryBody ) == 0 )
                        {
                            singleAccelerationUpdateNeeds[ body_transational_state_update ].push_back(
                                        primaryBody );
                        }
                    }
                    break;
                }
                case direct_tidal_dissipation_acceleration:
                    singleAccelerationUpdateNeeds[ body_rotational_state_update ].push_back(
                                accelerationModelIterator->first );
                    singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].
                            push_back( accelerationModelIterator->first );
                    break;
                case empirical_acceleration:
                    break;
                default:
                    throw std::runtime_error( std::string( "Error when setting acceleration model update needs, model type not recognized: " ) +
                                              std::to_string( currentAccelerationModelType ) );
                    break;
                }

            }

            // Check whether requested updates are possible.
            checkValidityOfRequiredEnvironmentUpdates( singleAccelerationUpdateNeeds, bodyMap );

            // Add requested updates of current acceleration model to
            // full list of environment updates.
            addEnvironmentUpdates( environmentModelsToUpdate, singleAccelerationUpdateNeeds );
        }
    }

    return environmentModelsToUpdate;
}

//! Get list of required environment model update settings from mass rate models.
std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >
createMassPropagationEnvironmentUpdaterSettings(
        const std::map< std::string, std::vector< boost::shared_ptr< basic_astrodynamics::MassRateModel > > > massRateModels,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    using namespace basic_astrodynamics;
    using namespace propagators;

    std::map< propagators::EnvironmentModelsToUpdate,
            std::vector< std::string > > environmentModelsToUpdate;
    std::map< propagators::EnvironmentModelsToUpdate,
            std::vector< std::string > > singleRateModelUpdateNeeds;

    // Iterate over all bodies with mass rate model.
    for( std::map< std::string, std::vector< boost::shared_ptr< MassRateModel > > >::const_iterator massRateModelIterator =
         massRateModels.begin( ); massRateModelIterator != massRateModels.end( ); massRateModelIterator++ )
    {
        for( unsigned int i = 0; i < massRateModelIterator->second.size( ); i++ )
        {
            singleRateModelUpdateNeeds.clear( );

            // Identify mass rate type and set required environment update settings.
            AvailableMassRateModels currentAccelerationModelType =
                    getMassRateModelType( massRateModelIterator->second.at( i ) );
            switch( currentAccelerationModelType )
            {
            case custom_mass_rate_model:
                break;
            case from_thrust_mass_rate_model:
                break;
            default:
                throw std::runtime_error( std::string( "Error when setting mass rate model update needs, model type not recognized: " ) +
                                          std::to_string( currentAccelerationModelType ) );

            }

            // Check whether requested updates are possible.
            checkValidityOfRequiredEnvironmentUpdates( singleRateModelUpdateNeeds, bodyMap );

            // Add requested updates of current acceleration model to
            // full list of environment updates.
            addEnvironmentUpdates( environmentModelsToUpdate, singleRateModelUpdateNeeds );
        }
    }

    return environmentModelsToUpdate;
}

//! Function to update environment to allow all required updates to be made
void checkAndModifyEnvironmentForDependentVariableSaving(
        const EnvironmentModelsToUpdate updateType,
        const boost::shared_ptr< SingleDependentVariableSaveSettings > dependentVariableSaveSettings,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    switch( updateType )
    {
    case vehicle_flight_conditions_update:
        if( bodyMap.at( dependentVariableSaveSettings->associatedBody_ )->getFlightConditions( ) == NULL )
        {
            if( ( bodyMap.at( dependentVariableSaveSettings->secondaryBody_ )->getAtmosphereModel( ) ) != NULL &&
                    ( bodyMap.at( dependentVariableSaveSettings->associatedBody_ )->getAerodynamicCoefficientInterface( ) != NULL ) )
            {
                bodyMap.at( dependentVariableSaveSettings->associatedBody_ )->setFlightConditions(
                            simulation_setup::createAtmosphericFlightConditions(
                                bodyMap.at( dependentVariableSaveSettings->associatedBody_ ),
                                bodyMap.at( dependentVariableSaveSettings->secondaryBody_ ),
                                dependentVariableSaveSettings->associatedBody_,
                                dependentVariableSaveSettings->secondaryBody_ ) );
            }
            else
            {
                bodyMap.at( dependentVariableSaveSettings->associatedBody_ )->setFlightConditions(
                            simulation_setup::createFlightConditions(
                                bodyMap.at( dependentVariableSaveSettings->associatedBody_ ),
                                bodyMap.at( dependentVariableSaveSettings->secondaryBody_ ),
                                dependentVariableSaveSettings->associatedBody_,
                                dependentVariableSaveSettings->secondaryBody_ ) );
            }
        }
        break;
    default:
        break;
    }
}

//! Function to create environment update settings for a single dependent variable
std::map< propagators::EnvironmentModelsToUpdate,
std::vector< std::string > > createEnvironmentUpdaterSettingsForDependentVariables(
        const boost::shared_ptr< SingleDependentVariableSaveSettings > dependentVariableSaveSettings,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >  variablesToUpdate;
    switch( dependentVariableSaveSettings->dependentVariableType_ )
    {
    case mach_number_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case altitude_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case airspeed_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case local_density_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case relative_speed_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case relative_position_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case relative_distance_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case relative_velocity_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case total_acceleration_norm_dependent_variable:
        break;
    case single_acceleration_norm_dependent_variable:
        break;
    case total_acceleration_dependent_variable:
        break;
    case single_acceleration_dependent_variable:
        break;
    case aerodynamic_force_coefficients_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case aerodynamic_moment_coefficients_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case rotation_matrix_to_body_fixed_frame_variable:
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        break;
    case intermediate_aerodynamic_rotation_matrix_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case relative_body_aerodynamic_orientation_angle_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case body_fixed_airspeed_based_velocity_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case total_aerodynamic_g_load_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case stagnation_point_heat_flux_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case local_temperature_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case geodetic_latitude_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case body_fixed_groundspeed_based_velocity_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case total_mass_rate_dependent_variables:
        break;
    case total_torque_norm_dependent_variable:
        break;
    case single_torque_norm_dependent_variable:
        break;
    case total_torque_dependent_variable:
        break;
    case single_torque_dependent_variable:
        break;
    case keplerian_state_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case modified_equinocial_state_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case spherical_harmonic_acceleration_terms_dependent_variable:
        break;
    case body_fixed_relative_cartesian_position:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case body_fixed_relative_spherical_position:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        variablesToUpdate[ body_rotational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case lvlh_to_inertial_frame_rotation_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case control_surface_deflection_dependent_variable:
        variablesToUpdate[ vehicle_flight_conditions_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        break;
    case radiation_pressure_dependent_variable:
        variablesToUpdate[ radiation_pressure_interface_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    case periapsis_altitude_dependent_variable:
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->associatedBody_ );
        variablesToUpdate[ body_translational_state_update ].push_back( dependentVariableSaveSettings->secondaryBody_ );
        break;
    default:
        throw std::runtime_error( "Error when getting environment updates for dependent variables, parameter " +
                                  std::to_string( dependentVariableSaveSettings->dependentVariableType_ ) + " not found." );
    }

    if( variablesToUpdate.count( vehicle_flight_conditions_update ) > 0 )
    {
        checkAndModifyEnvironmentForDependentVariableSaving(
                    vehicle_flight_conditions_update, dependentVariableSaveSettings, bodyMap );
    }

    return variablesToUpdate;
}

//! Create environment update settings for dependent variables
std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > createEnvironmentUpdaterSettings(
        const boost::shared_ptr< DependentVariableSaveSettings > dependentVariableSaveSettings,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > environmentModelsToUpdate;

    if( dependentVariableSaveSettings != NULL )
    {
        std::vector< boost::shared_ptr< SingleDependentVariableSaveSettings > > dependentVariableList =
                dependentVariableSaveSettings->dependentVariables_;
        for( unsigned int i = 0; i < dependentVariableList.size( ); i++ )
        {
            std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > currentEnvironmentModelsToUpdate =
                    createEnvironmentUpdaterSettingsForDependentVariables( dependentVariableList.at( i ), bodyMap );
            addEnvironmentUpdates( environmentModelsToUpdate, currentEnvironmentModelsToUpdate );
        }
    }
    return environmentModelsToUpdate;
}

//! Create environment update settings for termination settings
std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > createEnvironmentUpdaterSettings(
        const boost::shared_ptr< PropagationTerminationSettings > terminationSettings,
        const simulation_setup::NamedBodyMap& bodyMap )
{
    std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > > environmentModelsToUpdate;
    switch( terminationSettings->terminationType_ )
    {
    case time_stopping_condition:
        break;
    case cpu_time_stopping_condition:
        break;
    case dependent_variable_stopping_condition:
    {
        boost::shared_ptr< PropagationDependentVariableTerminationSettings > dependentVariableTerminationSettings =
                boost::dynamic_pointer_cast< PropagationDependentVariableTerminationSettings >( terminationSettings );
        std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >
                environmentModelsToUpdateForSingleTerminationSetting =
                createEnvironmentUpdaterSettingsForDependentVariables(
                    dependentVariableTerminationSettings->dependentVariableSettings_, bodyMap );
        addEnvironmentUpdates( environmentModelsToUpdate, environmentModelsToUpdateForSingleTerminationSetting );
        break;
    }
    case hybrid_stopping_condition:
    {
        boost::shared_ptr< PropagationHybridTerminationSettings > hybridTerminationSettings =
                boost::dynamic_pointer_cast< PropagationHybridTerminationSettings >( terminationSettings );
        for( unsigned int i = 0; i < hybridTerminationSettings->terminationSettings_.size( ); i++ )
        {
            std::map< propagators::EnvironmentModelsToUpdate, std::vector< std::string > >
                    environmentModelsToUpdateForSingleTerminationSetting = createEnvironmentUpdaterSettings(
                        hybridTerminationSettings->terminationSettings_.at( i ), bodyMap );
            addEnvironmentUpdates( environmentModelsToUpdate, environmentModelsToUpdateForSingleTerminationSetting );
        }
        break;
    }
    default:
        throw std::runtime_error( "Error when creating environment updater settings for termination conditions, type not found" );
    }

    return environmentModelsToUpdate;
}

//! Function to create 'brute-force' update settings, in which each environment model is updated.
std::map< propagators::EnvironmentModelsToUpdate,
std::vector< std::string > > createFullEnvironmentUpdaterSettings(
        const simulation_setup::NamedBodyMap& bodyMap )
{
    using namespace basic_astrodynamics;
    using namespace electro_magnetism;
    using namespace gravitation;
    using namespace propagators;

    std::map< propagators::EnvironmentModelsToUpdate,
            std::vector< std::string > > environmentModelsToUpdate;
    std::map< propagators::EnvironmentModelsToUpdate,
            std::vector< std::string > > singleAccelerationUpdateNeeds;

    // Iterate over all bodies.
    for( simulation_setup::NamedBodyMap::const_iterator
         bodyIterator = bodyMap.begin( ); bodyIterator != bodyMap.end( ); bodyIterator++ )
    {
        singleAccelerationUpdateNeeds.clear( );

        // Check if current body is a vehicle.

        // Check if current body has flight conditions set.
        if( bodyIterator->second ->getFlightConditions( ) != NULL )
        {
            // If vehicle has flight conditions, add flight conditions update function to update list.
            singleAccelerationUpdateNeeds[ vehicle_flight_conditions_update ].
                    push_back( bodyIterator->first );
        }

        // Get body radiation pressure interface(s) (one per source)
        std::map< std::string,
                boost::shared_ptr< RadiationPressureInterface > > radiationPressureInterfaces
                = bodyIterator->second->getRadiationPressureInterfaces( );

        // Add each interface update function to update list.
        for( std::map< std::string, boost::shared_ptr< RadiationPressureInterface > >::iterator
             iterator = radiationPressureInterfaces.begin( );
             iterator != radiationPressureInterfaces.end( ); iterator++ )
        {
            singleAccelerationUpdateNeeds[ radiation_pressure_interface_update ].
                    push_back( bodyIterator->first );
        }

        // If body has rotation model, update rotational state in each time step.;
        if( ( bodyIterator->second->getRotationalEphemeris( ) != NULL ) ||
                ( bodyIterator->second->getDependentOrientationCalculator( ) != NULL ) )
        {
            singleAccelerationUpdateNeeds[ body_rotational_state_update ].
                    push_back( bodyIterator->first );
        }


        boost::shared_ptr< TimeDependentSphericalHarmonicsGravityField > gravityField =
                boost::dynamic_pointer_cast< TimeDependentSphericalHarmonicsGravityField >
                ( bodyIterator->second->getGravityFieldModel( ) );
        if( gravityField != NULL )
        {
            singleAccelerationUpdateNeeds[ spherical_harmonic_gravity_field_update ].
                    push_back( bodyIterator->first );
        }

        singleAccelerationUpdateNeeds[ body_mass_update ].push_back( bodyIterator->first );

        // Check whether requested updates are possible.
        checkValidityOfRequiredEnvironmentUpdates( singleAccelerationUpdateNeeds, bodyMap );

        // Add requested updates of current acceleration model to full list of environment updates.
        addEnvironmentUpdates( environmentModelsToUpdate, singleAccelerationUpdateNeeds );
    }
    return environmentModelsToUpdate;
}

} // namespace propagators

} // namespace tudat
