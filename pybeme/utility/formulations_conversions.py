
def from_fx_to_fy(simr: "Simulator", y: int) -> "Simulator":
    """
    Convert the formulation from "bevarmejo::anytown::type::fx" to "bevarmejo::anytown::type::fy"
    """
    pre_fx = simr.data['problem']['type'].split('::')[:-1] # Track the main family of formulations

    assert pre_fx[0] == 'bevarmejo', 'The formulation is not from the bevarmejo namespace'

    assert pre_fx[1] == 'anytown', 'The formulation is not a Anytown problem'

    fx = simr.data['problem']['type'].split('::')[-1] # Extract the formulation type

    assert y > 0 and isinstance(y, int), 'The formulation type must be an integer greater than 0. Then each type has a different number of possible formulations'
    fy = 'f'+str(y)

    # Here there should be a table (if necessary that, based on the type and the starting and end formulation calls the right method.

    # From f1 to f2 or f3 (and viceversa), we need to change the existing pipes and the name
    # From f2 to f3 (and viceversa), nothing to do. Just replace the name.
    
    # the different between formulation 1 and 2 is how the decision option for the existing pipes is handled.
    # In f1 is a 2-value conditional decision, in f2 and f3 is one value decision.
    # 1. F1: [0: do-nothing, 1: clean, 2: duplicate] x [0->9: existing pipe rehabilitation alternative (active only if 2)]
    # 2. F2/F3: [0: do-nothing, 1: clean, 2->11 duplicate with this existing pipe rehabilitation alternative]

    if fx == 'f1' and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):
        simr.data['problem']['type'] = simr.data['problem']['type'].replace(fx, fy)


        assert len(simr.data['problem']['parameters']['at_subnets']['existing_pipes']) == 35, 'There shuold be 35 existing pipes'
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

    elif fy == 'f1' and (pre_fx[2] == 'mixed' or pre_fx[2] == 'rehab'):
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

    else:
        simr.data['problem']['type'] = simr.data['problem']['type'].replace(fx, fy)

    return simr
