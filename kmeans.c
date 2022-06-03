#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

static PyObject* fit(PyObject* self, PyObject* args);
void opt(int K, double** dataPoints, double** centeroids, double** clusters, int* clusters_size, int max_iter, int N, int d, double epsilon);
void init_clusters(double ** clusters,int K, int d, int* sizes);
double* update_centeroid(double *cluster, int cluster_size, int d);
int find_min(double* dataPoint, double** centeroids, int K, int d);
double calculate_dist(double* point1, double* point2, int d);
void sum_vectors(double* vect1, double* vect2, int d);
int decimal(char* c);
double ** getMatrix(int N, int d);
void freeMatrix(double** m, int k);

static PyObject* fit(PyObject* self, PyObject* args)
{
    Py_ssize_t i, j, i2, j2;
    int N, d, K, max_iter, p, q;
    double epsilon;
    PyObject *centeroids_indexesP,**centeroidsP, **datapointsP, *item;
    double **centeroids, **datapoints, **clusters;
    int *cluster_sizes;
    
    if(!PyArg_ParseTuple(args, "iiiidOOO:fit", &N, &d, &K, &max_iter,&epsilon, &centeroids_indexesP, &centeroidsP, &datapointsP))
    {
        return NULL;
    }

    datapoints = getMatrix(N,d);
    if (datapoints == NULL){
        return Py_BuildValue("");
    }
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < d; j++)
        {
            datapoints[i][j] = PyFloat_AS_DOUBLE(PyList_GetItem(PyList_GetItem(datapointsP, i),j));
            if (PyErr_Occurred())
            {
            puts("An error has occured");
            } 
        }
    }
    centeroids = getMatrix(K, d);
    if (centeroids == NULL){
        freeMatrix(centeroids ,K);
        return Py_BuildValue("");
    }
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < d; j++)
        {
            centeroids[i][j] = PyFloat_AS_DOUBLE(PyList_GetItem(PyList_GetItem(datapointsP, i),j));
            if (PyErr_Occurred())
            {
            puts("An error has occured");
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
    opt(K, datapoints, centeroids, clusters, cluster_sizes, max_iter, N, d, epsilon);

   return Py_BuildValue("O", centeroids);

}

static PyMethodDef capiMethods[] = {
    {"fit",
    (PyCFunction) fit,
    METH_VARARGS,
    PyDoc_STR("mykmeanssp")},
    {NULL, NULL, 0, NULL}
};

static struct PyModuleDef mykmeanssp = { 
    PyModuleDef_HEAD_INIT, 
   "mykmeanssp",
   NULL,
   -1,
   capiMethods 
};

PyMODINIT_FUNC
PyInit_mykmeanssp(void)
{
    PyObject* kmeans;
    kmeans = PyModule_Create(&mykmeanssp);
    if(!kmeans)
    {
        return NULL;
    }
    return kmeans;
}

void opt(int K, double** dataPoints, double** centeroids, double** clusters, int* clusters_size, int max_iter, int N, int d, double epsilon)
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

double ** getMatrix(int N, int d)
{
    double **data;
    int i, j;
    data = (double**) calloc(N, sizeof(double*));
    if(data == NULL)
        return data;
        for(i = 0; i < N; i++)
        {
            data[i] = (double*)calloc(d,sizeof(double));
            if(data[i] == NULL)
            {
                for(j = 0 ;j <= i-1; j++)
                {
                    free(data[j]);
                }
            }
            free(data);
            return NULL;
        }
        return data;
}

void freeMatrix(double** m, int k) {
    int i;
    for (i=0; i<k; i++) {
        free(m[i]);
    }
    free(m);
}
