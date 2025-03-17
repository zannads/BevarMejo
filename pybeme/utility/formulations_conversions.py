
def from_fx_to_fy(simr: "Simulator", y: int) -> "Simulator":
    """
    Convert the formulation from "bevarmejo::anytown::type::fx" to "bevarmejo::anytown::type::fy"
    """
    pre_fx = simr.data['problem']['type'].split('::')[:-1] # Track the main family of formulations

    assert pre_fx[0] == 'bevarmejo', 'The formulation is not from the bevarmejo namespace'

    assert pre_fx[1] == 'anytown', 'The formulation is not a Anytown problem'

    fx = simr.data['problem']['type'].split('::')[-1] # Extract the formulation type

    assert y > 1 and isinstance(y, int), 'The formulation type must be an integer greater than 1. Then each type has a different number of possible formulations'
    fy = 'f'+str(y)

    # Here there should be a table (if necessary that, based on the type and the starting and end formulation calls the right method.

    # For now only the default swap is implemented
    
    simr.data['problem']['type'] = '::'.join(pre_fx+[fy])

    return simr
