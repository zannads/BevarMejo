import numpy as np

def naive_pareto_front(data: np.array, f__indexes=False) -> np.array:
    """
    Finds the Pareto front using a naive approach (O(n^2*m))

    Args:
        data: A numpy array representing points in m-dimensional space.

    Returns:
        A numpy array containing the Pareto front points.
    """
    pareto_front = []
    for i in range(len(data)):
        dominated = False
        for j in range(len(data)):
            if i != j:
                # Check dominance in each objective dimension
                all_better = np.all(data[i] > data[j])
                if all_better:
                    dominated = True
                    break
        if not dominated:
            if f__indexes:
                pareto_front.append(i)
            else:
                pareto_front.append(data[i])
    return np.array(pareto_front)

# Example usage
data = [[1, 2], [3, 1], [2, 3], [4, 0]]
pareto_front = naive_pareto_front(data)
print("Naive Pareto Front:", pareto_front)
