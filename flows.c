
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REQUIRED_FLOW_FIELDS 7 //required arguments count
#define IPV4_OCTETS 4 //required ipv4 count
#define IPV4_MAX 255 //max possible ip
#define IPV4_MIN 0 //min possible ip part
#define START_FLOWS_IN_CLUSTER 1 //start amount

typedef struct {
    char *soubor;
    int N;
    double WB;
    double WT;
    double WD;
    double WS;
} Parametrs; //type of parametrs

typedef struct
{
    int flow_id;
    int *ip_source_parts;
    int *ip_dest_parts;
    int total_bytes;
    int flow_duration;
    int packet_count;
    double avg_interarrival_time;
} Flow_data; //type of flow
typedef struct
{
    Flow_data *data;
    int flows_count;
} Flow; //type of flow_lists
typedef struct
{
    Flow_data *flow_data;
    int flows_in_cluster;
} Cluster; //type of clusters

bool ip_check(int *ip_parts) //checking if ip parts are fine
{
    for (int i = 0; i < IPV4_OCTETS; i++) {
        if ((ip_parts[i] < 0) || (ip_parts[i] > 255)) {
            return false; //if not
        }
    }
    return true; //if yes
}
Flow read_file(FILE *file_ptr) //reading file function
{
    int count; //amount of floats
    fscanf(file_ptr, "count=%d", &count); //reading count
    Flow flows;
    flows.data = malloc(sizeof(Flow_data) * count); //initialization dynamic memory for flows
    int i = 0;
    while (i < count) { //while not the end of amount of floats
        fscanf(file_ptr, "%d", &flows.data[i].flow_id); // reading id
        flows.data[i].ip_source_parts = malloc(sizeof(int) * IPV4_OCTETS); //dynamic memmory for source ip
        fscanf(file_ptr, "%d.%d.%d.%d", &flows.data[i].ip_source_parts[0], &flows.data[i].ip_source_parts[1],
               &flows.data[i].ip_source_parts[2], &flows.data[i].ip_source_parts[3]); // reading source ip
        flows.data[i].ip_dest_parts = malloc(sizeof(int) * IPV4_OCTETS); //dynamic memmory for dest ip
        fscanf(file_ptr, "%d.%d.%d.%d", &flows.data[i].ip_dest_parts[0], &flows.data[i].ip_dest_parts[1],
               &flows.data[i].ip_dest_parts[2], &flows.data[i].ip_dest_parts[3]); //reading dest ip
        fscanf(file_ptr, "%d", &flows.data[i].total_bytes);            // reading total bytes
        fscanf(file_ptr, "%d", &flows.data[i].flow_duration);          // reading flow duration
        fscanf(file_ptr, "%d", &flows.data[i].packet_count);           // reading packet count
        fscanf(file_ptr, "%lf", &flows.data[i].avg_interarrival_time); // reading average interarrival time
        if (!ip_check(flows.data[i].ip_source_parts) || !(ip_check(flows.data[i].ip_dest_parts)) || flows.data[i].total_bytes == '\0' || flows.data[i].flow_duration == '\0' || flows.data[i].packet_count == '0' || flows.data[i].avg_interarrival_time == '\0') { //if something is wrong
            free(flows.data[i].ip_source_parts); //freeing dynamic memmory of ip
            free(flows.data[i].ip_dest_parts); 
            count--; //-1 normal flow
            i--; //reinitialization starts
        }
        i++; //going
    }
    flows.flows_count = count;
    return flows;
}
void *cluster_ctor(Flow flows) //initialization of cluster
{
    Cluster *cluster = malloc(sizeof(Cluster) * flows.flows_count);
    for (int i = 0; i < flows.flows_count; i++) {
        cluster[i].flows_in_cluster = START_FLOWS_IN_CLUSTER;
        cluster[i].flow_data = malloc(sizeof(Flow_data) * cluster[i].flows_in_cluster);
        cluster[i].flow_data[cluster->flows_in_cluster - 1] = flows.data[i];
    }
    return cluster;
}
void flows_dtor(Flow flows) //destructor of flow
{
    for (int i = 0; i < flows.flows_count; i++) {
        free(flows.data[i].ip_source_parts);
        free(flows.data[i].ip_dest_parts);
    }
    free(flows.data);
    flows.data = NULL;
}
void clusters_dtor(Cluster *cluster, int *cluster_count) //destructor of cluster
{
    for (int i = 0; i < *cluster_count; i++) {
        free(cluster[i].flow_data);
    }
    free(cluster);
    cluster = NULL;
}

double euclidian_distance(Flow_data flow_data1, Flow_data flow_data2, Parametrs p)
{
    double s1 = 0.0;
    if (flow_data1.packet_count > 0) {
        s1 = (double)flow_data1.total_bytes / flow_data1.packet_count; //delka paketu 1
    }
    double s2 = 0.0;
    if (flow_data2.packet_count > 0) {
        s2 = (double)flow_data2.total_bytes / flow_data2.packet_count; //delka paketu 2
    }
    double db = flow_data1.total_bytes - flow_data2.total_bytes; //total bytes
    double dt = flow_data1.flow_duration - flow_data2.flow_duration; // flow duration
    double dd = flow_data1.avg_interarrival_time - flow_data2.avg_interarrival_time; // avgerage interarrival time
    double ds = s1 - s2;
    double distance = sqrt(
        p.WB * db * db +
        p.WT * dt * dt +
        p.WD * dd * dd +
        p.WS * ds * ds
    ); //distance counting
    return distance;
}

void cluster_delete(Cluster* c, int index, int *cluster_count) { //deleting cluster from clusters
    free(c[index].flow_data);
    for (int i = index; i < *cluster_count - 1; i++) {
        c[i] = c[i + 1]; 
    }
    (*cluster_count)--;
}

void clusterization(Cluster *c, Parametrs *p, int *cluster_count)
{
    while (*cluster_count > p->N) { //while not amount of clusters that we want
    int index1 = 0;
    int index2 = 1;
    double min_distance = INFINITY; //maximum
    for (int i = 0; i < *cluster_count - 1; i++) { //every cluster 1
        for (int j = i + 1; j < *cluster_count; j++) { //every cluster 2
            for (int k = 0; k < c[i].flows_in_cluster; k++) { //every flow 1
                for (int l = 0; l < c[j].flows_in_cluster; l++) { //every flow 2
                    double distance = euclidian_distance(c[i].flow_data[k], c[j].flow_data[l], *p); //checking distance
                    if (distance < min_distance ){ //if less than min
                        min_distance = distance;
                        index1 = i; //saving index
                        index2 = j;
                    } 
                }
            }
        }
    }
    int old_size = c[index1].flows_in_cluster;
    c[index1].flows_in_cluster += c[index2].flows_in_cluster;
    c[index1].flow_data = realloc(c[index1].flow_data, sizeof(Flow_data) * c[index1].flows_in_cluster); //resizing cluster

    for (int k = 0; k < c[index2].flows_in_cluster; k++) {
        c[index1].flow_data[old_size + k] = c[index2].flow_data[k];
    } //collapsing clusters

    cluster_delete(c, index2, cluster_count);
}
}

int compare_flow(const void* a, const void* b) { //function for qsort of id in flows
    const Flow_data* fa = (const Flow_data*)a;
    const Flow_data* fb = (const Flow_data*)b;
    return (fa->flow_id - fb->flow_id);
}

int compare_cluster(const void* a, const void* b) { //function for qsort of id in cluster
    const Cluster* ca = (const Cluster*)a;
    const Cluster* cb = (const Cluster*)b;

    if (ca->flows_in_cluster == 0 && cb->flows_in_cluster == 0) return 0; //if something doesn't fit right
    if (ca->flows_in_cluster == 0) return 1;
    if (cb->flows_in_cluster == 0) return -1;

    return (ca->flow_data[0].flow_id - cb->flow_data[0].flow_id); //value for qsort
}

void cluster_sort(Cluster* c, int cluster_count) {
    for (int i = 0; i < cluster_count; i++) { // in every flow
        qsort(c[i].flow_data, c[i].flows_in_cluster, sizeof(Flow_data), compare_flow); //qsorting id's in every flow
    }

    qsort(c, cluster_count, sizeof(Cluster), compare_cluster); //qsorting flow_id in clusters
}

Parametrs *param_ctor(void) //initialization of empty parametrs structure
{
    Parametrs *p = malloc(sizeof(Parametrs));
    p->N = '\0';
    p->soubor = NULL;
    p->WB = '\0';
    p->WD = '\0';
    p->WS = '\0';
    p->WT = '\0';
    return p;
}
int param_analitics(Parametrs *p, char *argv[], int argc) //reading params
{
    for (int i = 0; i < argc - 1; i++) { //if something does not fit right
        if (i > 1) {
            if (atoi(argv[i]) < 0) {
                printf("Nevalidní váhy."); 
                return -1;
            }
        }
    }
    if (argv[1] == NULL) {
        return -1;
    }
    p->soubor = malloc(strlen(argv[1]) + 1); //dynamic memory for file name
    strcpy(p->soubor, argv[1]);
    p->N = atoi(argv[2]);
    p->WB = atof(argv[3]);
    p->WT = atof(argv[4]);
    p->WD = atof(argv[5]);
    p->WS = atof(argv[6]); //reading params
    if (p->N != '\0' && argc != REQUIRED_FLOW_FIELDS) {
        printf("Nevalidní argumenty");
        return -1; //if somethind does not fit right
    }
    if (p->N == '\0') {
        return -1; //if somethind does not fit right
    }
    return 1; //if everything is fine
}
void print_all_clusters(Cluster *c, int *cluster_count) //printing of all clusters
{
    printf("Clusters: \n");
    for (int i = 0; i < *cluster_count; i++) { //every cluster
        printf("cluster %d: ", i);
        for (int j = 0; j < c[i].flows_in_cluster; j++) { //every flow in cluster
            printf("%d ", c[i].flow_data[j].flow_id);
        }
        printf("\n");
    }
}
void param_dtor(Parametrs *p) //destruktor of params
{
    free(p->soubor);
    free(p);
    p = NULL;
}

int main(int argc, char *argv[])
{

    FILE *fptr;
    Flow flows;
    Cluster *c;
    Parametrs *params = param_ctor(); //initialization of structs
    if (param_analitics(params, argv, argc) < 0) {
        param_dtor(params);
        return -1;
    } //if sometning wrong with analytics of params
    fptr = fopen(argv[1], "r");
    if (fptr == NULL) { //if someting wrong with opening file
        printf("Prazdny soubor");
        return -1;
    }
    flows = read_file(fptr);
    if (flows.flows_count <= 0) { //if something wrong with flow count in file
        printf("Nevalidni vstupni flow");
        return -1;
    }
    c = cluster_ctor(flows); //initialization of cluster
    int cluster_count = flows.flows_count; //at the beginning in every cluster only one flow. So basicly count of clusters = count of flows
    clusterization(c, params, &cluster_count); // volani funkce
    cluster_sort(c, cluster_count); //qsort of cluster
    print_all_clusters(c, &cluster_count); //printing the result of clusterization and sorting
    clusters_dtor(c, &cluster_count); //freeing dynamic memory of clusters
    flows_dtor(flows); //freeing dynamic memory of clusters
    param_dtor(params); //freeing dynamic memory of clusters
    fclose(fptr); //closing file
    return 0; //hura
}
