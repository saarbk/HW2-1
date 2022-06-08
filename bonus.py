from sklearn.datasets import load_iris
from sklearn.cluster import KMeans
import matplotlib.pyplot as plt
import pandas as pd

if __name__ == '__main__':
    '''
                  ||  Bonus Section ||
         Implement sklearn KMeans using elbow method

    '''
    data = pd.DataFrame(load_iris().data, columns=[1, 2, 3, 4])
    model = {}

    for k in range(1, 10):
        k_mean_Model = KMeans(init='k-means++', random_state=0, n_clusters=k).fit(data)
        model[k] = k_mean_Model.inertia_ / 150

    # Plotting the results, allowing us to observe 'The elbow'
    fig = plt.subplot()
    fig.add_patch(plt.Circle((3, model[3]), 0.2, color='r', fill=False))
    fig.annotate('Elbow point', xy=(3, model[3] + 0.1), xycoords='data',
                 xytext=(0.4, 0.3), textcoords='axes fraction',
                 arrowprops=dict(facecolor='black', shrink=0.2),
                 horizontalalignment='right',
                 )
    plt.plot(list(model.keys()), list(model.values()), 'gx-')
    plt.xlabel('Values of K')
    plt.ylabel('Average Dispersion')
    plt.title('Elbow Method for selection of optimal "K" cluster')
    plt.savefig('elbow.png')
    plt.show()
