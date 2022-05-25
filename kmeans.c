#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static PyObject* fit(PyObject* self, PyObject* args);
double** opt(int K, double** dataPoints, double** centeroids, double** clusters, int* clusters_size, int max_iter, int N, int d, double epsilon);
void init_clusters(double ** clusters,int K, int d, int* sizes);
double* update_centeroid(double *cluster, int cluster_size, int d);
int find_min(double* dataPoint, double** centeroids, int K, int d);
double calculate_dist(double* point1, double* point2, int d);
void sum_vectors(double* vect1, double* vect2, int d);
int decimal(char* c);

static PyMethodDef capiMethods[] = {
    {"fit",
    (PyCFunction) fit,
    METH_VARARGS,
    PyDoc_STR("kmeans")},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef kmeans = { 
    PyModuleDef_HEAD_INIT, 
   "kmeans",
   NULL,
   -1,
   capiMethods 
};

PyMODINIT_FUNC
PyInit_kmeans(void)
{
    PyObject* kmeans;
    kmeans = PyModule_Create(&kmeans);
    if(!kmeans)
    {
        return NULL;
    }
    return kmeans;
}

double** opt(int K, double** dataPoints, double** centeroids, double** clusters, int* clusters_size, int max_iter, int N, int d, double epsilon)
{
    int reps = 0, done = 0, i, min_cluster, j;
    double* new_centeroid;
    new_centeroid = (double*) calloc(d, sizeof(double));

    while(done == 0 && reps < max_iter)
    {
        init_clusters(clusters, K, d, clusters_size);
        done = 1;
        for(i = 0; i < N; i++)
        {
            min_cluster = find_min(dataPoints[i], centeroids, K, d);
            sum_vectors(clusters[min_cluster], dataPoints[i], d);
            clusters_size[min_cluster]++;
        }
        for (i = 0; i < K; i++)
        {
            if(clusters_size[i] > 0)
                new_centeroid = update_centeroid(clusters[i], clusters_size[i], d);

            if(calculate_dist(new_centeroid, centeroids[i], d) >= epsilon)
                done = 0;
            for(j = 0; j < d; j++)
            {
                centeroids[i][j] = new_centeroid[j];
            }
        }
        reps++;
    }
    free(new_centeroid);
    return centeroids;
}
double* update_centeroid(double *cluster, int cluster_size, int d)
{
    int i;
    for (i = 0; i < d; i++)
    {
        cluster[i] = (double)(cluster[i] / cluster_size);
    }
    return cluster;
}

int find_min(double* dataPoint, double** centeroids, int K, int d)
{
    int index_min = 0, i;
    double min, dist;
    min = calculate_dist(centeroids[0], dataPoint, d);
    for(i = 1; i < K; i++)
    {
        dist = calculate_dist(centeroids[i], dataPoint, d);
        if(dist < min)
        {
            index_min = i;
            min = dist;
        }
    }
    return index_min;
}
double calculate_dist(double* point1, double* point2, int d)
{
    int i;
    double sum;
    sum = 0.0;
    for(i = 0; i < d; i++)
    {
        sum += pow((point1[i] - point2[i]) , 2);
    }
    sum = pow(sum, 0.5);
    return sum;
}

void sum_vectors(double* vect1, double* vect2, int d)
{
    int i;
    for(i = 0; i < d; i++)
    {
        vect1[i] += vect2[i];
    }
}

void init_clusters(double ** clusters,int K, int d, int* sizes)
{
    int i,j;
    for(i = 0; i < K; i++)
    {
        sizes[i] = 0;
        for(j = 0; j < d; j++)
        {
            clusters[i][j] = 0.0;
        }
    }
}

int decimal(char* c)
{
    int i = 0;
    if(c[0] == '-')
        i++;
    for(; c[i] != 0; i++)
    {
        if(c[i] > '9' || c[i] < '0')
            return 0;
    }
    return 1;
}
static PyObject* fit(PyObject* self, PyObject* args)
{
    Py_ssize_t i, j;
    int N, d, K, max_iter, p, q;
    double epsilon;
    PyObject *centeroids_indexesP,**centeroidsP, **datapointsP, *item;
    double **centeroids, **datapoints, **clusters;
    int *cluster_sizes;
    
    if(!PyArg_ParseTuple(args, "iiiidooo:fit", &N, &d, &K, &max_iter, &centeroids_indexesP, &centeroidsP, &datapointsP))
    {
        return NULL;
    }

    datapoints = (double*)malloc(N);
    for (i = 0; i < N; i++)
    {
        datapoints[i] = (double*)malloc(d);
        for (j = 0; j < d; j++)
        {
            item = PyList_GetItem(PyList_GetItem(datapointsP, j),i);
            datapoints[i][j] = PyFloat_AS_DOUBLE(item);
            if (PyErr_Occurred())
            {
            puts("Something bad ...");
            free(datapoints[p]);
            } 
        }
    }
    
    centeroids = (double*)malloc(K);
    for (i = 0; i < K; i++)
    {
        centeroids[i] = (double*)malloc(d);
        for (j = 0; j < d; j++)
        {
           item = PyList_GetItem(PyList_GetItem(centeroidsP, j),i);
           centeroids[i][j] = PyFloat_AS_DOUBLE(item);
           if (PyErr_Occurred())
           {
            puts("Something bad ...");
            free(datapoints[p]);
            }
        }
    }
    clusters = (double**) calloc(K, sizeof(double*));
    cluster_sizes = (int*) calloc(K, sizeof(int));

    for (q = 0; q < K; q++)
    {
        for(p = 0; p < d; p++)
        {
            clusters[q][p] = centeroids[q][p];
        }
        cluster_sizes[q] = 1;
    }
   return Py_BuildValue("o", opt(K, datapoints, centeroids, clusters, cluster_sizes, max_iter, N, d, epsilon));
}