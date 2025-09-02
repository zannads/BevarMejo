import math

def anytown__from_fx_to_fy(simr: "Simulator", y: int) -> "Simulator":
    """
    Convert the formulation from "bevarmejo::anytown::type::fx" to "bevarmejo::anytown::type::fy"
    """
    pre_fx = simr.data['problem']['type'].split('::')[:-1] # Track the main family of formulations

    assert pre_fx[0] == 'bevarmejo', 'The formulation is not from the bevarmejo namespace'

    assert pre_fx[1] == 'anytown', 'The formulation is not a Anytown problem'

    fx = simr.data['problem']['type'].split('::')[-1] # Extract the formulation type

    assert y > 0 and isinstance(y, int) and y <6, 'The formulation type must be an integer greater than 0. Then each type has a different number of possible formulations'
    fy = 'f'+str(y)

    # Here there should be a table (if necessary that, based on the type and the starting and end formulation calls the right method.

    # From f1 to f2 or f3 or f4 (and viceversa), we need to change the existing pipes and the name
    # From f2 to f3 or f4 (and viceversa), nothing to do because only the objective function changes. Just replace the name.
    # From f1 to f5, we need to change the existing pipes, the tank and the name. The viceversa is not possible because the parameters are fixed in f1.
    # From f2 or f3 or f4 to f5, we only need to change the existing pipes and the name. The viceversa is not possible because the parameters are fixed in f1.
    
    # the different between formulation 1 and 2 is how the decision option for the existing pipes is handled.
    # In f1 is a 2-value conditional decision, in f2 and f3 is one value decision.
    # 1. F1: [0: do-nothing, 1: clean, 2: duplicate] x [0->9: existing pipe rehabilitation alternative (active only if 2)]
    # 2. F2/F3/F4: [0: do-nothing, 1: clean, 2->11 duplicate with this existing pipe rehabilitation alternative]

    if (fx == 'f1') and (fy in ['f2', 'f3', 'f4']) and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):
        simr.data['problem']['type'] = simr.data['problem']['type'].replace(fx, fy)

        assert len(simr.data['problem']['parameters']['at_subnets']['existing_pipes']) == 35, 'There should be 35 existing pipes'
        dvs_old = simr.data['decision_vector']
        dvs_new = []
        for i in range(len(simr.data['problem']['parameters']['at_subnets']['existing_pipes'])):
            if dvs_old[i*2] < 2:
                dvs_new.append(dvs_old[i*2])
            else:
                dvs_new.append(dvs_old[i*2+1] + 2)
        
        for i in range(len(simr.data['problem']['parameters']['at_subnets']['existing_pipes'])*2, len(dvs_old)):
            dvs_new.append(dvs_old[i])

        simr.data['decision_vector'] = dvs_new

    elif (fx == 'f1') and (fy == 'f5') and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):
        # let's go from f1 to f2 and then from f2 to f5
        return from_fx_to_fy( from_fx_to_fy(simr, 2), 5)
    
    elif (fx in ['f2', 'f3', 'f4']) and (fy == 'f1') and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):
        simr.data['problem']['type'] = simr.data['problem']['type'].replace(fx, fy)

        assert len(simr.data['problem']['parameters']['at_subnets']['existing_pipes']) == 35, 'There should be 35 existing pipes'
        dvs_old = simr.data['decision_vector']
        dvs_new = []
        for i in range(len(simr.data['problem']['parameters']['at_subnets']['existing_pipes'])):
            if dvs_old[i] < 2:
                dvs_new.append(dvs_old[i])
                dvs_new.append(0)
            else:
                dvs_new.append(2)
                dvs_new.append(dvs_old[i]-2)
        
        for i in range(len(simr.data['problem']['parameters']['at_subnets']['existing_pipes']), len(dvs_old)):
            dvs_new.append(dvs_old[i])

        simr.data['decision_vector'] = dvs_new

    elif (fx in ['f2', 'f3', 'f4']) and (fy == 'f5') and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):

        # While in f1/2/3/4/5 the decision variable for the new tanks was a 2-value with (i) the tank location with the do-nothing alternative,
        # and (ii) the tank volume option.
        # In f5, the decision variable is a  6 values with (i) the diameter of the riser (with do-nothing option), (ii) the tank location,
        # (iii) the tank diameter, (iv) the tank hmax, (v) the tank hmin, and (vi) the tank safety level.
        # In the f1/2/3/4, there were some additional assumptions. E.g., elevation = 215 ft (as in the other exisisting tank),
        # min level or safety level 10 ft, (as in the other existing tank), and the tank diameter was equal to the max level (which is from the elevation).
        
        simr.data['problem']['type'] = simr.data['problem']['type'].replace(fx, fy)

        # pop the last 4 decision variables (twice the location and the volume of the tank)
        old_dvs_unchanged = simr.data['decision_vector'][:-4]
        old_dvs_tank = simr.data['decision_vector'][-4:]
        # also track which are continuous and which are discrete. Until f4 they were all discrete (hence False)
        dvs_new = old_dvs_unchanged[:]
        continuous_dvs = [False for i in range(len(dvs_new))]
        
        # add the location and the volume of the tank
        for t in range(2):    
            if old_dvs_tank[t*2] == 0 or ( t>0 and old_dvs_tank[t*2] == old_dvs_tank[(t-1)*2]):
                for i in range(6):
                    dvs_new.append(0)
            else:
                # 
                assert len(simr.data['problem']['parameters']['new_pipe_options']) == 10, 'There should be 10 new pipes options'
                # (1) The first one in f5 is the diameter of the riser, which was fixed at 16 inches in the previous ones.
                # So, we extract the index of the pipe alternative cost and add 1 to account for the 0, i.e. , the  do-nothing alternative
                found = False
                for i, npo in enumerate(simr.data['problem']['parameters']['new_pipe_options']):
                    if npo['diameter__in'] == 16:
                        dvs_new.append(i+1)
                        found = True
                        break
                if not found:
                    raise ValueError('The diameter of the riser was not found in the new pipe options')

                # (2) The second one to insert in f5 is the location, which was the first one in f1/f2/f3/f4
                # We simply remove the do-nothing alternative and use the value as is
                assert len(simr.data['problem']['parameters']['at_subnets']['possible_tank_locations']) == 17, 'There should be 35 existing pipes'
                dvs_new.append(old_dvs_tank[t*2]-1)

                # (3) tank diameter and the rest...
                # The assumption was that in f1/f2/f3/f4 was that the tank diameter was equal to the operational levels height
                # The selected volume was the operational one, not accounting for the safety level.
                vol__opt = int(old_dvs_tank[t*2+1])
                
                # convert from option to volume in gallons, then to m3
                assert len(simr.data['problem']['parameters']['tank_options']) == 5, 'There should be 5 tank options'
                vol__gal = simr.data['problem']['parameters']['tank_options'][vol__opt]['volume__gal']
                vol__m3 = vol__gal * 0.00378541

                # convert to diameter inverting the volume formula for a cylindrical tank with d equalt to h
                diam__m = (4*vol__m3/math.pi) ** (1/3)

                elev = 65.532 # m -> 215 ft
                hmin = 68.58 # m -> 225 ft
                hmax = elev + diam__m
                safety_level = 3.048 # m -> 10 ft
                
                dvs_new.append(diam__m)
                dvs_new.append(hmax)
                dvs_new.append(hmin)
                dvs_new.append(safety_level)

            # Add which are continuous and which are discrete. This is independent of the values
            continuous_dvs.append(False) # for the selection of the diam riser
            continuous_dvs.append(False) # for the selection of the tank location
            continuous_dvs.append(True) # diam
            continuous_dvs.append(True) # hmax
            continuous_dvs.append(True) # hmin
            continuous_dvs.append(True) # safety lev

        # We need to reorder the decision vector with the continuous variables at the beginning
        reordered_dvs = []
        # First iterate to order the continuous
        for i, is_dv_continuous in enumerate(continuous_dvs):
            if (is_dv_continuous):
                reordered_dvs.append(dvs_new[i])
        
        # Then do the same for the discretes...
        for i, is_dv_continuous in enumerate(continuous_dvs):
            if (is_dv_continuous == False):
                reordered_dvs.append(dvs_new[i])

        simr.data['decision_vector'] = reordered_dvs
        return simr

    elif (fx == 'f5') and (fy in ['f1', 'f2', 'f3', 'f4']) and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):
        raise ValueError('The conversion from f5 to f1/f2/f3/f4 is not possible. The tank parameters that can be optimized in f5 are fixed in these other formulations.')
    
    else:
        simr.data['problem']['type'] = simr.data['problem']['type'].replace(fx, fy)

    return simr

def anytown_systol25__from_a_to_b(simr: "Simulator", b: str) -> "Simulator":
    pre_f = simr.data['problem']['type'].split('::')[:-1] # Track the main family of formulations

    assert pre_f[0] == 'bevarmejo', 'The formulation is not from the bevarmejo namespace'
    assert pre_f[1] == 'anytown_systol25', 'The formulation is not a Anytown problem'

    a = simr.data['problem']['type'].split('::')[-1] # Extract the formulation type
    assert b in ['hyd_rel', 'mec_rel', 'fire_rel'], 'Formulation to convert from for the anytown_systol problem not recognised '
    assert b in ['hyd_rel', 'mec_rel', 'fire_rel'], 'Formulation to convert to for the anytown_systol problem not recognised '

    if a == b:
        return simr
    
    # Generally, I would just need to change the name with replace(a,b)
    # However:
    # 1. from hyd_rel to the others I need to remove the operations from the dec vect and viceversa add them.
    # 2. from any to the fire_rel I need to add the fireflow file

    if a == 'hyd_rel':
        # Assign the operations (last 24 dvs) as a problem parameter and remove them from the dv
        simr.data['problem']['parameters']['pump_group_operations'] = simr.data['decision_vector'][-24:]
        simr.data['decision_vector'] = simr.data['decision_vector'][:-24]
    
    if b == 'hyd_rel':
        # Add the operations as additional dvs (as taken from the cpp anytown_systol)
        simr.data['decision_vector'].extend(
           [3, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2,
            2, 3, 3, 3, 3, 3,
            3, 3, 3, 3, 3, 2]
        )

    if b == 'fire_rel':
        # Add the fireflow inp file
        simr.data['problem']['parameters']['anytown_fireflow_inp'] = "anytown_base_exeter-ff-30min.inp"

    # Conclude by switching the formulations
    simr.data['problem']['type'] = simr.data['problem']['type'].replace(a, b)
    return simr