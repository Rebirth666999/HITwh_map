#ifdef _MSC_VER
// ���MSVC��������Ԥ����ָ��
#pragma comment(linker, "/SUBSYSTEM:WINDOWS")  // ָ��ΪWindows��ϵͳ
#pragma comment(linker, "/ENTRY:mainCRTStartup") // ������ڵ�Ϊmain
#pragma comment(lib, "legacy_stdio_definitions.lib") // ���Ӵ�ͳ��׼��
#pragma comment(lib, "gdi32.lib")  // ����GDIͼ�ο�
#pragma comment(lib, "user32.lib")  // �����û������
#pragma comment(lib, "kernel32.lib") // �����ں˿�
#pragma comment(lib, "msimg32.lib") // ���Ӹ߼�ͼ�ο�
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h> // �ļ�·�������
#pragma comment(lib, "shlwapi.lib") // ����·�������

// ����Ϣ�ṹ��
typedef struct {
    int src;    // Դ����ID
    int dest;   // Ŀ�궥��ID
} Edge;

// ������Ϣ�ṹ��
typedef struct {
    int id;             // ����Ψһ��ʶ��
    char name[50];      // ��������
    char description[100]; // ��������
    int x, y;           // �����ڵ�ͼ�ϵ�����
} Building;

// �ڽӱ�ڵ㣨����·�����ˣ�
typedef struct AdjListNode {
    int dest;           // Ŀ�꽨��ID
    int weight;         // ·��Ȩ�أ����룩
    struct AdjListNode* next; // ��һ���ڵ�ָ��
} AdjListNode;

// �ڽӱ�ͷ�ڵ�
typedef struct {
    int building_id;    // ����ID
    AdjListNode* head;  // �ڽӱ�ͷָ��
} AdjListHead;

// ͼ�ṹ
typedef struct {
    int V;              // ��������
    int E;              // ������
    AdjListHead* array; // �ڽӱ�����
    Edge* edges;        // ������
} Graph;

// ��ϣ��ڵ�
typedef struct HashNode {
    int key;            // ����ID
    Building building;  // ��������
    struct HashNode* next; // ��һ���ڵ�ָ��
} HashNode;

// ��ϣ��ṹ
typedef struct {
    int size;           // ��ϣ���С
    HashNode** table;   // ��ϣͰ����
} HashTable;

#define HASH_SIZE 100     // ��ϣ���С
#define MAX_BUILDINGS 100 // ���������
#define WINDOW_WIDTH 600  // ���ڿ��
#define WINDOW_HEIGHT 900 // ���ڸ߶�
#define DATA_FILENAME _T("campus_data.txt") // �����ļ���
#define EDGE_FILENAME _T("campus_edges.txt")

// ȫ�ֱ�������
Building buildings[MAX_BUILDINGS]; // ��������
Graph* campus_graph = NULL;        // У԰·��ͼ
HashTable* building_table = NULL;  // ������ϣ��
static int* shortest_path = NULL;  // ���·������
static int path_size = 0;          // ·������
HBITMAP campus_map = NULL;         // У԰��ͼλͼ
HWND hwnd;                         // �����ھ��
static bool is_edit_mode = false;  // �༭ģʽ��־
static int selected_building_id = -1; // ��ǰѡ�еĽ���ID
static int edit_edge_mode = 0;       // 0:�޲��� 1:��ӱ� 2:ɾ����
static int first_building_id = -1;   // �߱༭�ĵ�һ������ID

// ��������
Graph* createGraph(int V, int E);
void addEdge(Graph* graph, int src, int dest, int weight, int edge_index);
HashTable* createHashTable(int size);
int hash(int key);
void hashInsert(HashTable* hashtable, Building building);
Building* hashSearch(HashTable* hashtable, int key);
void dijkstra(Graph* graph, int src, int* dist, int* prev);
int* getShortestPath(int* prev, int dest, int* path_size);
void loadCampusMap();
void initializeCampusData();
void drawCampusMap(HDC hdc);
void drawPath(HDC hdc, int* path, int path_size);
void showBuildingInfo(HDC hdc, int building_id);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void saveBuildingData();  // ���潨������
void loadBuildingData();  // ���ؽ�������
void createDefaultBuildings(); // ����Ĭ�Ͻ�������
void createDefaultEdges(Graph** graph); // ����Ĭ�ϱ�����
void saveEdgeData(Graph* graph); // ���������
void cleanupResources(); // ������Դ


/**
 * ����Ĭ�Ͻ�������
 */
void createDefaultBuildings() {
    // ��ʼ��Ϊ��Чֵ
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        buildings[i].id = -1;
    }

    // ������Ч��������
    Building b0 = { 0, "��¥", "���񴦡���ʦ�칫��", 415, 580 };
    Building b1 = { 1, "ͼ���", "ͼ���������ϰ����������", 235,100 };
    Building b2 = { 2, "��ѧ¥", "����", 220,172 };
    Building b3 = { 3, "��ѧ¥", "����", 262,562 };
    Building b4 = { 4, "��ѧ¥", "����", 318,552 };
    Building b5 = { 5, "��ѧ¥", "����", 320,502 };
    Building b6 = { 6, "��ѧ¥", "����", 477,507 };
    Building b7 = { 7, "�о�Ժ", "����ʵ����", 441,750 };
    Building b8 = { 8, "�ﾶ��", "�ﾶ��", 266,611 };
    Building b9 = { 9, "����", "����", 215,650 };
    Building b10 = { 10, "�ֻ���", "�ֻ���", 335,590 };
    Building b11 = { 11, "����", "����", 335,632 };
    Building b12 = { 12, "����", "����", 335,676 };
    Building b13 = { 13, "����", "����", 288,679 };
    Building b14 = { 14, "�ۺ������", "��ë��ƹ��������", 366,171 };
    Building b15 = { 15, "��չѵ����", "��չ�˶�", 461,151 };
    Building b16 = { 16, "��������", "��������", 398,191 };
    Building b17 = { 17, "����ѵ����", "����ѵ����", 397,243 };
    Building b18 = { 18, "̽��¥", "����ѧԺ", 182,78 };
    Building b19 = { 19, "��ѧ�������", "���ҡ��赸��", 250,172 };
    Building b20 = { 20, "ѧ�Ӳ���", "ʳ�ã���4�㣩", 241,290 };
    Building b21 = { 21, "ѧԷ����", "ʳ�ã���2�㣩", 328,387 };
    Building b22 = { 22, "1��Ԣ", "ѧ������", 336,429 };
    Building b23 = { 23, "2��Ԣ", "ѧ������", 265,437 };
    Building b24 = { 24, "3��Ԣ", "ѧ������", 265,403 };
    Building b25 = { 25, "4��Ԣ", "ѧ������", 262,375 };
    Building b26 = { 26, "5��Ԣ", "ѧ������", 338,468 };
    Building b27 = { 27, "6��Ԣ", "ѧ������", 260,335 };
    Building b28 = { 28, "7��Ԣ", "ѧ������", 179,434 };
    Building b29 = { 29, "8��Ԣ", "ѧ������", 203,352 };
    Building b30 = { 30, "9��Ԣ", "ѧ������", 203,310 };
    Building b31 = { 31, "10��Ԣ", "ѧ������", 156,309 };
    Building b32 = { 32, "11��Ԣ", "ѧ������", 111,309 };
    Building b33 = { 33, "12��Ԣ", "ѧ������", 466,289 };
    Building b34 = { 34, "13��Ԣ", "ѧ������", 532,286 };
    Building b35 = { 35, "14��Ԣ", "ѧ������", 542,209 };
    Building b36 = { 36, "����", "ѧУ����", 413,814 };
    Building b37 = { 37, "������", "������", 96,185 };
    Building b38 = { 38, "����", "����", 576,570 };
    Building b39 = { 39, "���²�ҵ԰", "У����ҵ", 51,307 };
    Building b40 = { 40, "Уʷ��", "����У����ʷ", 385,445 };
    Building b41 = { 41, "��ѧ����������", "���á����ꡢ��ӡ�ҡ�ϴ����", 262,488 };
    Building b42 = { 42, "����Է", "�Ƶ�", 465,238 };
    Building b43 = { 43, "��ѧ����Ԣ", "��ѧ����Ԣ", 465,202 };
    Building b44 = { 44, "���վ", "���վ", 314,294 };
    Building b45 = { 45, "test", "test", 56,456 };


    // ���浽ȫ������
    buildings[0] = b0;
    buildings[1] = b1;
    buildings[2] = b2;
    buildings[3] = b3;
    buildings[4] = b4;
    buildings[5] = b5;
    buildings[6] = b6;
    buildings[7] = b7;
    buildings[8] = b8;
    buildings[9] = b9;
    buildings[10] = b10;
    buildings[11] = b11;
    buildings[12] = b12;
    buildings[13] = b13;
    buildings[14] = b14;
    buildings[15] = b15;
    buildings[16] = b16;
    buildings[17] = b17;
    buildings[18] = b18;
    buildings[19] = b19;
    buildings[20] = b20;
    buildings[21] = b21;
    buildings[22] = b22;
    buildings[23] = b23;
    buildings[24] = b24;
    buildings[25] = b25;
    buildings[26] = b26;
    buildings[27] = b27;
    buildings[28] = b28;
    buildings[29] = b29;
    buildings[30] = b30;
    buildings[31] = b31;
    buildings[32] = b32;
    buildings[33] = b33;
    buildings[34] = b34;
    buildings[35] = b35;
    buildings[36] = b36;
    buildings[37] = b37;
    buildings[38] = b38;
    buildings[39] = b39;
    buildings[40] = b40;
    buildings[41] = b41;
    buildings[42] = b42;
    buildings[43] = b43;
    buildings[44] = b44;
    buildings[45] = b45;

    // ���һ����Ч�������ǽ������Ǳ��룬������һ���ԣ�
    // ʵ���ϣ������Ѿ���ʼ��������λ��Ϊ-1�����Բ���Ҫ��������
}

/**
 * ����Ĭ�ϱ�����
 */
void createDefaultEdges(Graph** graph) {
    // Ԥ���������
    int edges[][2] = {
        {0, 7}, {0, 38}, {0, 6}, {0, 40},   // ��¥(0) -> �о�Ժ(7), ����(38), ��ѧ¥(6), Уʷ��(40)
        {1, 18}, {1, 19},                    // ͼ���(1) -> ̽��¥(18), ��ѧ�������(19)
        {2, 19}, {2, 37},                    // ��ѧ¥(2) -> ��ѧ�������(19), ������(37)
        {3, 4}, {3, 8}, {3, 41},             // ��ѧ¥(3) -> ��ѧ¥(4), �ﾶ��(8), ��ѧ����������(41)
        {4, 5}, {4, 10},                     // ��ѧ¥(4) -> ��ѧ¥(5), �ֻ���(10)
        {5, 26},                              // ��ѧ¥(5) -> 5��Ԣ(26)
        {6, 7}, {6, 40}, {6, 33},            // ��ѧ¥(6) -> �о�Ժ(7), Уʷ��(40), 12��Ԣ(33)
        {7, 36},                              // �о�Ժ(7) -> ����(36)
        {8, 9}, {8, 10}, {8, 3},             // �ﾶ��(8) -> ����(9), �ֻ���(10), ��ѧ¥(3)
        {10, 11},                             // �ֻ���(10) -> ����(11)
        {11, 12},                             // ����(11) -> ����(12)
        {12, 13},                             // ����(12) -> ����(13)
        {14, 15}, {14, 16}, {14, 19},        // �ۺ������(14) -> ��չѵ����(15), ��������(16), ��ѧ�������(19)
        {15, 16}, {15, 43},                  // ��չѵ����(15) -> ��������(16), ��ѧ����Ԣ(43)
        {16, 17},                             // ��������(16) -> ����ѵ����(17)
        {17, 33}, {17, 44},                  // ����ѵ����(17) -> 12��Ԣ(33), ���վ(44)
        {18, 1},                              // ̽��¥(18) -> ͼ���(1)
        {19, 20}, {19, 2},                   // ��ѧ�������(19) -> ѧ�Ӳ���(20), ��ѧ¥(2)
        {20, 27}, {20, 30}, {20, 44},        // ѧ�Ӳ���(20) -> 6��Ԣ(27), 9��Ԣ(30), ���վ(44)
        {21, 22}, {21, 25},                  // ѧԷ����(21) -> 1��Ԣ(22), 4��Ԣ(25)
        {22, 26}, {22, 40},                  // 1��Ԣ(22) -> 5��Ԣ(26), Уʷ��(40)
        {23, 24}, {23, 41},                  // 2��Ԣ(23) -> 3��Ԣ(24), ��ѧ����������(41)
        {24, 25},                             // 3��Ԣ(24) -> 4��Ԣ(25)
        {25, 27},                             // 4��Ԣ(25) -> 6��Ԣ(27)
        {26, 22}, {26, 41},                  // 5��Ԣ(26) -> 1��Ԣ(22), ��ѧ����������(41)
        {27, 20}, {27, 25}, {27, 44},        // 6��Ԣ(27) -> ѧ�Ӳ���(20), 4��Ԣ(25), ���վ(44)
        {28, 24}, {28, 29},                  // 7��Ԣ(28) -> 3��Ԣ(24), 8��Ԣ(29)
        {29, 30},                             // 8��Ԣ(29) -> 9��Ԣ(30)
        {30, 31}, {30, 29},                  // 9��Ԣ(30) -> 10��Ԣ(31), 8��Ԣ(29)
        {31, 32},                             // 10��Ԣ(31) -> 11��Ԣ(32)
        {32, 37}, {32, 39},                  // 11��Ԣ(32) -> ������(37), ���²�ҵ԰(39)
        {33, 34}, {33, 35}, {33, 42}, {33, 17}, // 12��Ԣ(33) -> 13��Ԣ(34), 14��Ԣ(35), ����Է(42), ����ѵ����(17)
        {34, 35},                             // 13��Ԣ(34) -> 14��Ԣ(35)
        {35, 34},                             // 14��Ԣ(35) -> 13��Ԣ(34)
        {36, 7},                              // ����(36) -> �о�Ժ(7)
        {37, 32}, {37, 39},                  // ������(37) -> 11��Ԣ(32), ���²�ҵ԰(39)
        {38, 0},                              // ����(38) -> ��¥(0)
        {39, 32}, {39, 37},                  // ���²�ҵ԰(39) -> 11��Ԣ(32), ������(37)
        {40, 6}, {40, 33}, {40, 22}, {40, 0}, // Уʷ��(40) -> ��ѧ¥(6), 12��Ԣ(33), 1��Ԣ(22), ��¥(0)
        {41, 23}, {41, 26}, {41, 3},         // ��ѧ����������(41) -> 2��Ԣ(23), 5��Ԣ(26), ��ѧ¥(3)
        {42, 33}, {42, 43},                  // ����Է(42) -> 12��Ԣ(33), ��ѧ����Ԣ(43)
        {43, 15}, {43, 42},                  // ��ѧ����Ԣ(43) -> ��չѵ����(15), ����Է(42)
        {44, 27}, {44, 20}, {44, 17}         // ���վ(44) -> 6��Ԣ(27), ѧ�Ӳ���(20), ����ѵ����(17)
        
    };
    int num_edges = sizeof(edges) / sizeof(edges[0]);

    // ����ͼ��Ԥ����num_edges����
    *graph = createGraph(MAX_BUILDINGS, num_edges);
    if (!*graph) {
        return;
    }

    int actual_edge_count = 0; // ʵ����ӵı���

    for (int i = 0; i < num_edges; i++) {
        int src = edges[i][0];
        int dest = edges[i][1];
        Building* b1 = hashSearch(building_table, src);
        Building* b2 = hashSearch(building_table, dest);

        // ���������������������Ч
        if (b1 && b2 && b1->id != -1 && b2->id != -1) {
            // ����Ȩ��
            int dx = b1->x - b2->x;
            int dy = b1->y - b2->y;
            int weight = (int)sqrt(dx * dx + dy * dy);
            // ��ӱߣ�ʹ��actual_edge_count��Ϊ��ǰ�ߵ�������
            addEdge(*graph, src, dest, weight, actual_edge_count);
            actual_edge_count++;
        }
    }

    // ����ͼ�ı�����Ϊʵ����ӵı���
    (*graph)->E = actual_edge_count;
}

 /**
  * ����ͼ�ṹ
  * @param V ��������
  * @param E ������
  * @return ������ͼָ�룬ʧ�ܷ���NULL
  */
Graph* createGraph(int V, int E) {
    Graph* graph = (Graph*)malloc(sizeof(Graph));
    if (!graph) return NULL;

    graph->V = V;
    graph->E = E;  // ��ʼ��������
    graph->array = (AdjListHead*)malloc(V * sizeof(AdjListHead));
    graph->edges = (Edge*)malloc(E * sizeof(Edge));  // ����������ڴ�

    if (!graph->array || !graph->edges) {
        if (graph->array) free(graph->array);
        if (graph->edges) free(graph->edges);
        free(graph);
        return NULL;
    }

    // ��ʼ��ÿ��������ڽӱ�
    for (int i = 0; i < V; ++i) {
        graph->array[i].building_id = i;
        graph->array[i].head = NULL;
    }

    return graph;
}

/**
 * ��ͼ����ӱߣ�·����
 * @param graph ͼָ��
 * @param src Դ����ID
 * @param dest Ŀ�궥��ID
 * @param weight ·��Ȩ�أ����룩
 */
void addEdge(Graph* graph, int src, int dest, int weight, int edge_index) {
    // �������Ϣ
    if (edge_index < graph->E) {
        graph->edges[edge_index].src = src;
        graph->edges[edge_index].dest = dest;
    }

    // ����Դ���㵽Ŀ�궥��ı�
    AdjListNode* newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!newNode) return;

    newNode->dest = dest;
    newNode->weight = weight;
    newNode->next = graph->array[src].head;
    graph->array[src].head = newNode;

    // ����ͼ������Ŀ�궥�㵽Դ����ı�
    newNode = (AdjListNode*)malloc(sizeof(AdjListNode));
    if (!newNode) return;

    newNode->dest = src;
    newNode->weight = weight;
    newNode->next = graph->array[dest].head;
    graph->array[dest].head = newNode;
}

/**
 * ���±ߵ�Ȩ��
 * @param graph ͼָ��
 * @param building_id ���µĽ���ID
 */
void updateEdgeWeight(Graph* graph, int building_id) {
    // �������б�
    for (int i = 0; i < graph->E; i++) {
        Edge e = graph->edges[i];

        // ����߰����ý���
        if (e.src == building_id || e.dest == building_id) {
            Building* b1 = hashSearch(building_table, e.src);
            Building* b2 = hashSearch(building_table, e.dest);

            if (b1 && b2) {
                // ����ŷ����þ���
                int dx = b1->x - b2->x;
                int dy = b1->y - b2->y;
                int weight = (int)sqrt(dx * dx + dy * dy);

                // ����Դ��Ŀ��ı�
                AdjListNode* node = graph->array[e.src].head;
                while (node) {
                    if (node->dest == e.dest) {
                        node->weight = weight;
                        break;
                    }
                    node = node->next;
                }

                // ����Ŀ�굽Դ�ıߣ�����ͼ��
                node = graph->array[e.dest].head;
                while (node) {
                    if (node->dest == e.src) {
                        node->weight = weight;
                        break;
                    }
                    node = node->next;
                }
            }
        }
    }
}

/**
 * ���ر�����
 */
void loadEdgeData(Graph** graph) {
    TCHAR szPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szPath);
    TCHAR fullPath[MAX_PATH];
    _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), szPath, EDGE_FILENAME);

    if (!PathFileExists(fullPath)) {
        // �ļ������ڣ�����Ĭ�ϱ�
        createDefaultEdges(graph);
        // �������浽�ļ���ʵ�ֺͽڵ�һ�µĳ־û��߼�
        if (*graph) {
            saveEdgeData(*graph);
        }
        return;
    }

    FILE* file = NULL;
    if (_tfopen_s(&file, fullPath, _T("r")) != 0 || file == NULL) {
        // �ļ���ʧ�ܣ�����Ĭ�ϱ�
        createDefaultEdges(graph);
        // �������浽�ļ���ʵ�ֺͽڵ�һ�µĳ־û��߼�
        if (*graph) {
            saveEdgeData(*graph);
        }
        return;
    }

    // ͳ�Ʊ�����
    int edge_count = 0;
    int src, dest;
    while (fscanf_s(file, "%d,%d\n", &src, &dest) == 2) {
        edge_count++;
    }
    rewind(file);

    // ����ͼ�ṹ
    *graph = createGraph(MAX_BUILDINGS, edge_count);
    if (!*graph) {
        fclose(file);
        return;
    }

    // ���ر�����
    int index = 0;
    while (fscanf_s(file, "%d,%d\n", &src, &dest) == 2 && index < edge_count) {
        Building* b1 = hashSearch(building_table, src);
        Building* b2 = hashSearch(building_table, dest);

        // ���NULL���
        if (!b1 || !b2 || b1->id == -1 || b2->id == -1) {
            continue;  // ������Ч����
        }

        // ����Ȩ�أ�ŷ����þ��룩
        int dx = b1->x - b2->x;
        int dy = b1->y - b2->y;
        int weight = (int)sqrt(dx * dx + dy * dy);

        addEdge(*graph, src, dest, weight, index);
        index++;
    }

    fclose(file);
}


/**
 * ������ϣ��
 * @param size ��ϣ���С
 * @return �����Ĺ�ϣ��ָ�룬ʧ�ܷ���NULL
 * ����Ͱ���鲢��ʼ��
 */
HashTable* createHashTable(int size) {
    // 1. �����ϣ��ṹ�ڴ�
    HashTable* hashtable = (HashTable*)malloc(sizeof(HashTable));
    if (!hashtable) return NULL;

    // 2. ���ù�ϣ���С
    hashtable->size = size;

    // 3. ����Ͱ�����ڴ�
    hashtable->table = (HashNode**)malloc(sizeof(HashNode*) * size);
    if (!hashtable->table) {
        free(hashtable);
        return NULL;
    }

    // 4. ��ʼ������ͰΪ��
    for (int i = 0; i < size; i++) {
        hashtable->table[i] = NULL;
    }

    return hashtable;
}

/**
 * ��ϣ����
 * @param key ����ID
 * @return ��ϣֵ
 */
int hash(int key) {
    return key % HASH_SIZE;
}

/**
 * ���ϣ����뽨������
 * @param hashtable ��ϣ��ָ��
 * @param building ��������
 */
void hashInsert(HashTable* hashtable, Building building) {
    // 1. �����ϣֵ
    int index = hash(building.id);

    // 2. �����½ڵ�
    HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
    if (!new_node) return;

    // 3. ���ڵ�����
    new_node->key = building.id;
    new_node->building = building;

    // 4. ͷ�巨��������
    new_node->next = hashtable->table[index];
    hashtable->table[index] = new_node;
}

/**
 * �ڹ�ϣ���в��ҽ���
 * @param hashtable ��ϣ��ָ��
 * @param key ����ID
 * @return �ҵ��Ľ���ָ�룬δ�ҵ�����NULL
 */
Building* hashSearch(HashTable* hashtable, int key) {
    // 1. �����ϣֵ
    int index = hash(key);

    // 2. ��ȡͰ����ͷ
    HashNode* node = hashtable->table[index];

    // 3. �����������
    while (node != NULL && node->key != key) {
        node = node->next;
    }

    // 4. ���ؽ��
    if (node == NULL)
        return NULL;
    return &(node->building);
}

/**
��ϣ����Ҫ���ڿ��ٲ��Һ͸�����ڵ���ص�״̬��Ϣ�����롢ǰ�����������Ǵ洢���յ�·������·����ͨ��ǰ��ָ�������ݵõ��ġ�
*/

/**
 * Dijkstra���·���㷨
 * @param graph ͼ�ṹָ��
 * @param src ���ID
 * @param dist �������飨�����
 * @param prev ǰ���ڵ����飨�����
 */
void dijkstra(Graph* graph, int src, int* dist, int* prev) {
    if (!graph) return;  // ���ͼ��Ч�Լ��

    int V = graph->V;  // ��ȡʵ�ʶ�����
    int* visited = (int*)malloc(V * sizeof(int));
    if (!visited) return;

    // ��ʼ�������ǰ������
    for (int i = 0; i < V; i++) {
        dist[i] = INT_MAX;   // ��ʼ������Ϊ�����
        visited[i] = 0;      // δ���ʱ��
        prev[i] = -1;        // ��ǰ��
    }
    dist[src] = 0;          // ������Ϊ0

    // �������ж���
    for (int count = 0; count < V - 1; count++) {
        // ѡȡ��ǰ��С���붥��
        int min = INT_MAX, u = -1;
        for (int v = 0; v < V; v++) {
            if (!visited[v] && dist[v] <= min) {
                min = dist[v];
                u = v;
            }
        }

        if (u == -1 || min == INT_MAX) break; // û�пɴ�ڵ�
        visited[u] = 1;     // ���Ϊ�ѷ���

        // �����ڽӶ������
        AdjListNode* node = graph->array[u].head;
        while (node != NULL) {
            int v = node->dest;
            // ȷ��v����Ч��Χ��
            if (v >= 0 && v < V) {
                // ����ҵ�����·��
                if (!visited[v] && dist[u] != INT_MAX &&
                    dist[u] + node->weight < dist[v]) {
                    dist[v] = dist[u] + node->weight;
                    prev[v] = u;  // ����ǰ���ڵ�
                }
            }
            node = node->next;
        }
    }
    free(visited);
}

/**
 * ����ǰ�������ȡ���·��
 * @param prev ǰ���ڵ�����
 * @param dest �յ�ID
 * @param path_size ·�����ȣ������
 * @return ���·������ָ�룬���������ͷ��ڴ�
 */
int* getShortestPath(int* prev, int dest, int* path_size) {
    if (!prev) return NULL;

    int* temp_path = (int*)malloc(MAX_BUILDINGS * sizeof(int));
    if (!temp_path) return NULL;

    int count = 0;
    int current = dest;

    // ����׷��·��
    while (current != -1 && count < MAX_BUILDINGS) {
        temp_path[count++] = current;
        current = prev[current];
    }

    // �����������ڴ�
    int* result = (int*)malloc(count * sizeof(int));
    if (!result) {
        free(temp_path);
        return NULL;
    }

    // ��ת·��Ϊ����
    for (int i = 0; i < count; i++) {
        result[i] = temp_path[count - 1 - i];
    }

    *path_size = count;
    free(temp_path);
    return result;
}

/**
 * ����У԰��ͼ����
 * ֧��BMP��JPG��ʽ������BMP��
 */
void loadCampusMap() {
    TCHAR szPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szPath);  // ��ȡ��ǰ����Ŀ¼

    // ������ͼ�ļ�·��
    TCHAR fullPath[MAX_PATH];
    _stprintf_s(fullPath, MAX_PATH, _T(".\\res\\campus_map.bmp")); // ��ѡBMP��ʽ

    // ����ļ��Ƿ����
    if (!PathFileExists(fullPath)) {
        // ����JPG��ʽ
        _stprintf_s(fullPath, MAX_PATH, _T(".\\res\\campus_map.jpg"), szPath);
        if (!PathFileExists(fullPath)) {
            TCHAR szError[512];
            _stprintf_s(szError, 512,
                _T("��ͼ�ļ�������!\n·��: %s\n�����ļ��Ƿ������·����ȷ"),
                fullPath);
            MessageBox(hwnd, szError, _T("����"), MB_ICONERROR);
            exit(1);
        }
    }

    // ����λͼ�ļ�
    campus_map = (HBITMAP)LoadImage(
        NULL,                   // ��ָ��ʵ��
        fullPath,               // �ļ�·��
        IMAGE_BITMAP,           // λͼ����
        0, 0,                   // ��ָ�����
        LR_LOADFROMFILE | LR_CREATEDIBSECTION | LR_DEFAULTSIZE // ����ѡ��
    );

    if (!campus_map) {
        DWORD dwError = GetLastError();
        TCHAR szError[512];
        _stprintf_s(szError, 512,
            _T("�޷����ص�ͼ! �������: %d\n�ļ�·��: %s\n��ʾ: ���Խ�ͼƬת��Ϊ24λBMP��ʽ"),
            dwError, fullPath);
        MessageBox(hwnd, szError, _T("����"), MB_ICONERROR);
        exit(1);
    }
}

/**
 * ���潨�����ݵ��ļ�
 */
void saveBuildingData() {
    TCHAR szPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szPath);
    TCHAR fullPath[MAX_PATH];
    _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), szPath, DATA_FILENAME);

    FILE* file = NULL;
    if (_tfopen_s(&file, fullPath, _T("w")) != 0 || file == NULL) {
        TCHAR szError[512];
        _stprintf_s(szError, 512, _T("�޷����潨������! �ļ�·��: %s"), fullPath);
        MessageBox(hwnd, szError, _T("����"), MB_ICONERROR);
        return;
    }

    // д�������
    fprintf(file, "id,name,description,x,y\n");

    for (int i = 0; i < MAX_BUILDINGS; i++) {
        if (buildings[i].id == -1) continue;

        // ת�������еĶ��ţ��滻Ϊ�ֺţ�
        char escaped_desc[100];
        strcpy_s(escaped_desc, buildings[i].description);
        for (char* p = escaped_desc; *p; p++) {
            if (*p == ',') *p = ';';
        }

        // д��CSV��ʽ���ݣ�ʹ��խ�ַ���
        fprintf(file, "%d,%s,%s,%d,%d\n",
            buildings[i].id,
            buildings[i].name,
            escaped_desc,
            buildings[i].x,
            buildings[i].y);
    }

    fclose(file);
}

/**
 * ���ļ����ؽ�������
 */
void loadBuildingData() {
    // ��ʼ��Ϊ��Ч����
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        buildings[i].id = -1;
    }

    TCHAR szPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szPath);
    TCHAR fullPath[MAX_PATH];
    _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), szPath, DATA_FILENAME);

    if (!PathFileExists(fullPath)) {
        return; // �ļ�������
    }

    FILE* file = NULL;
    if (_tfopen_s(&file, fullPath, _T("r")) != 0 || file == NULL) {
        return;
    }

    // ����������
    TCHAR header[256];
    _fgetts(header, 256, file);

    int index = 0;
    while (!feof(file) && index < MAX_BUILDINGS) {
        int id, x, y;
        char name[50] = { 0 };  // ��ʼ��Ϊ0
        char desc[100] = { 0 }; // ��ʼ��Ϊ0

        // ʹ�� fscanf ��ȡխ�ַ�
        if (fscanf_s(file, "%d,%49[^,],%99[^,],%d,%d\n",
            &id, name, (unsigned)_countof(name),
            desc, (unsigned)_countof(desc), &x, &y) == 5) {
            buildings[index].id = id;

            // ȷ���ַ�����null��β
            name[49] = '\0';
            desc[99] = '\0';

            // ʹ�� strcpy_s ����խ�ַ���
            strcpy_s(buildings[index].name, sizeof(buildings[index].name), name);

            // ��ԭ�����е������ַ�
            for (char* p = desc; *p; p++) {
                if (*p == ';') *p = ',';
            }
            strcpy_s(buildings[index].description, sizeof(buildings[index].description), desc);

            buildings[index].x = x;
            buildings[index].y = y;
            index++;
        }
    }

    fclose(file);
}

/**
 * ��ʼ��У԰����
 * ����������Ϣ��·��ͼ�͹�ϣ��
 */
void initializeCampusData() {
    // ���Դ��ļ����ؽ�������
    loadBuildingData();

    // ����Ƿ�����Ч��������
    bool hasValidData = false;
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        if (buildings[i].id != -1) {
            hasValidData = true;
            break;
        }
    }

    // ���û����Ч���ݣ�����Ĭ������
    if (!hasValidData) {
        createDefaultBuildings();
        saveBuildingData();  // ����ΪTXT��ʽ
    }

    // ��ʼ����ϣ��
    building_table = createHashTable(HASH_SIZE);
    if (!building_table) {
        MessageBox(hwnd, L"��ϣ���ʼ��ʧ��!", L"����", MB_ICONERROR);
        return;
    }

    // ��������ӵ���ϣ��
    for (int i = 0; i < MAX_BUILDINGS; i++) {
        if (buildings[i].id != -1) {
            hashInsert(building_table, buildings[i]);
        }
    }

    // ���ر����ݣ��ᴴ��ͼ�ṹ��
    loadEdgeData(&campus_graph);

    // ����У԰��ͼ
    loadCampusMap();
}

void saveEdgeData(Graph* graph) {
    TCHAR szPath[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szPath);
    TCHAR fullPath[MAX_PATH];
    _stprintf_s(fullPath, MAX_PATH, _T("%s\\%s"), szPath, EDGE_FILENAME);

    FILE* file = NULL;
    if (_tfopen_s(&file, fullPath, _T("w")) != 0 || file == NULL) {
        return;
    }

    // �������б�
    for (int i = 0; i < graph->E; i++) {
        fprintf(file, "%d,%d\n", graph->edges[i].src, graph->edges[i].dest);
    }

    fclose(file);
}

/**
 * ANSIת���ַ���������
 * @param pszText ANSI�ַ���
 * @return ���ַ���ָ��
 */
static const wchar_t* CA2W(const char* pszText) {
    static wchar_t wszBuffer[512];
    if (pszText) {
        MultiByteToWideChar(CP_ACP, 0, pszText, -1, wszBuffer, _countof(wszBuffer));
    }
    else {
        wszBuffer[0] = L'\0';
    }
    return wszBuffer;
}

/**
 * ����У԰��ͼ
 * @param hdc �豸�����ľ��
 */
void drawCampusMap(HDC hdc) {
    // ���Ʊ�����ͼ
    if (campus_map) {
        HDC hdcMem = CreateCompatibleDC(hdc);
        if (hdcMem) {
            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, campus_map);
            BITMAP bm;
            GetObject(campus_map, sizeof(bm), &bm);

            // �������ű�����λ�ã����ֿ�߱ȣ�
            double scaleX = (double)WINDOW_WIDTH / bm.bmWidth;
            double scaleY = (double)WINDOW_HEIGHT / bm.bmHeight;
            double scale = scaleX < scaleY ? scaleX : scaleY; // ȡ��С��������֤������ʾ

            int scaledWidth = (int)(bm.bmWidth * scale);
            int scaledHeight = (int)(bm.bmHeight * scale);

            // �������λ��
            int offsetX = (WINDOW_WIDTH - scaledWidth) / 2;
            int offsetY = (WINDOW_HEIGHT - scaledHeight) / 2;

            // ʹ�����Ż���
            SetStretchBltMode(hdc, HALFTONE); // ����������
            StretchBlt(hdc,
                offsetX, offsetY, scaledWidth, scaledHeight,
                hdcMem,
                0, 0, bm.bmWidth, bm.bmHeight,
                SRCCOPY);

            SelectObject(hdcMem, hOldBitmap);
            DeleteDC(hdcMem);
        }
    }
    else {
        // �����ͼ����ʧ�ܣ�ʹ�ô�ɫ����
        HBRUSH hBrush = CreateSolidBrush(RGB(135, 206, 235)); // ����ɫ
        RECT rect = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        FillRect(hdc, &rect, hBrush);
        DeleteObject(hBrush);
    }

    // === ��������·������ɫ��===
    if (campus_graph) {
        // ������ɫ����
        HPEN grayPen = CreatePen(PS_SOLID, 2, RGB(180, 180, 180)); // ��ɫ·��
        if (grayPen) {
            HGDIOBJ hOldPen = SelectObject(hdc, grayPen);

            // ����ѻ��Ʊߵ�����
            int drawn[MAX_BUILDINGS][MAX_BUILDINGS] = { 0 };

            for (int u = 0; u < campus_graph->V; u++) {
                AdjListNode* node = campus_graph->array[u].head;
                while (node != NULL) {
                    int v = node->dest;

                    // ȷ��ֻ����һ�������
                    if (u < v && !drawn[u][v]) {
                        Building* b1 = hashSearch(building_table, u);
                        Building* b2 = hashSearch(building_table, v);

                        if (b1 && b2 && b1->id != -1 && b2->id != -1) {
                            // ���ƻ�ɫ·���߶�
                            MoveToEx(hdc, b1->x, b1->y, NULL);
                            LineTo(hdc, b2->x, b2->y);

                            // ��Ǹñ��ѻ���
                            drawn[u][v] = drawn[v][u] = 1;
                        }
                    }
                    node = node->next;
                }
            }

            SelectObject(hdc, hOldPen);
            DeleteObject(grayPen);
        }
    }

    // �������н������
    HBRUSH buildingBrush = CreateSolidBrush(RGB(173, 216, 230)); // ǳ��ɫ
    HPEN buildingPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));  // ��ɫ�߿�
    if (buildingBrush && buildingPen) {
        HGDIOBJ hOldBrush = SelectObject(hdc, buildingBrush);
        HGDIOBJ hOldPen = SelectObject(hdc, buildingPen);

        for (int i = 0; i < MAX_BUILDINGS; i++) {
            // ������Ч����
            if (buildings[i].id == -1) continue;

            // �༭ģʽ��ѡ�еĽ����û�ɫ���
            if (is_edit_mode && selected_building_id == i) {
                HBRUSH selectedBrush = CreateSolidBrush(RGB(255, 255, 0)); // ��ɫ
                SelectObject(hdc, selectedBrush);
                Ellipse(hdc, buildings[i].x - 20, buildings[i].y - 20,
                    buildings[i].x + 20, buildings[i].y + 20);
                DeleteObject(selectedBrush);
                SelectObject(hdc, buildingBrush);
            }
            else {
                // ���ƽ���Բ�α��
                Ellipse(hdc, buildings[i].x - 10, buildings[i].y - 10,
                    buildings[i].x + 10, buildings[i].y + 10);
            }

            // ��ʾ��������
            SetBkMode(hdc, OPAQUE); // ��͸������
            SetBkColor(hdc, RGB(255, 255, 255)); // ��ɫ����
            HFONT hNameFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                VARIABLE_PITCH, TEXT("΢���ź�"));
            if (hNameFont) {
                HFONT hOldFont = (HFONT)SelectObject(hdc, hNameFont);
                // �����������
                wchar_t wname[50];
                MultiByteToWideChar(CP_ACP, 0, buildings[i].name, -1, wname, 50);
                TextOut(hdc, buildings[i].x - 30, buildings[i].y + 20, wname, (int)wcslen(wname));
                SelectObject(hdc, hOldFont);
                DeleteObject(hNameFont);
            }
        }

        // �ָ���ɾ��GDI����
        SelectObject(hdc, hOldBrush);
        SelectObject(hdc, hOldPen);
        DeleteObject(buildingBrush);
        DeleteObject(buildingPen);
    }

    // ���ƹ���˵��
    SetBkMode(hdc, OPAQUE);
    SetBkColor(hdc, RGB(240, 240, 240)); // ǳ��ɫ����
    HFONT hInfoFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, TEXT("΢���ź�"));
    if (hInfoFont) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hInfoFont);
        TextOut(hdc, 20, WINDOW_HEIGHT - 60, TEXT("У԰��������˵��:"), (int)_tcslen(TEXT("У԰��������˵��:")));
        TextOut(hdc, 20, WINDOW_HEIGHT - 40, TEXT("1. ���ѡ����㣬�Ҽ�ѡ���յ�"),
            (int)_tcslen(TEXT("1. ���ѡ����㣬�Ҽ�ѡ���յ�")));
        TextOut(hdc, 20, WINDOW_HEIGHT - 20, TEXT("2. ���ո�����·������ESC�˳�"),
            (int)_tcslen(TEXT("2. ���ո�����·������ESC�˳�")));
        SelectObject(hdc, hOldFont);
        DeleteObject(hInfoFont);
    }

    // ���Ʋ����
    HBRUSH sidebarBrush = CreateSolidBrush(RGB(240, 240, 240)); // ǳ��ɫ
    if (sidebarBrush) {
        RECT sidebarRect = { 900, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
        FillRect(hdc, &sidebarRect, sidebarBrush);
        DeleteObject(sidebarBrush);
    }


    // ���Ʊ���
    SetBkMode(hdc, TRANSPARENT);
    HFONT hSideFont = CreateFont(35, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, TEXT("΢���ź�"));
    if (hSideFont) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hSideFont);
        SetTextColor(hdc, RGB(0, 0, 0)); // ��ɫ����
        TextOut(hdc, 25, 750, TEXT("������������У԰����"), (int)_tcslen(TEXT("������������У԰����")));
        SelectObject(hdc, hOldFont);
        DeleteObject(hSideFont);
    }

    // ���ƹ��ܰ�ť
    HFONT hButtonFont = CreateFont(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, TEXT("΢���ź�"));
    if (hButtonFont) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hButtonFont);
        SetTextColor(hdc, RGB(0, 0, 0)); // ��ɫ����

        // ���ư�ť����
        HBRUSH buttonBrush = CreateSolidBrush(RGB(200, 200, 255)); // ǳ��ɫ
        for (int i = 0; i < 2; i++) {
            RECT btnRect = { 190, 800 + i * 30, 300, 850 + i * 30 };
            FillRect(hdc, &btnRect, buttonBrush);
        }
        DeleteObject(buttonBrush);

        // ���ư�ť����
        TextOut(hdc, 200, 800, TEXT("�༭ģʽ"), (int)_tcslen(TEXT("�༭ģʽ")));
        TextOut(hdc, 200, 850, TEXT("�˳�ϵͳ"), (int)_tcslen(TEXT("�˳�ϵͳ")));

        SelectObject(hdc, hOldFont);
        DeleteObject(hButtonFont);
    }
}


 /**
  * �������·��������ͷ�������ǣ�
  */
void drawPath(HDC hdc, int* path, int path_size) {
    if (path_size < 2 || !path) return;

    // 1. ����·�����ʺͼ�ͷ��ˢ
    HPEN pathPen = CreatePen(PS_SOLID, 4, RGB(255, 0, 0)); // ��ɫʵ��
    HBRUSH arrowBrush = CreateSolidBrush(RGB(255, 0, 0));   // ��ɫ���
    if (!pathPen || !arrowBrush) return;

    // 2. Ӧ�û�ͼ����
    HGDIOBJ hOldPen = SelectObject(hdc, pathPen);
    HGDIOBJ hOldBrush = SelectObject(hdc, arrowBrush);

    // 3. ����·���߶κͼ�ͷ
    for (int i = 0; i < path_size - 1; i++) {
        Building* b1 = hashSearch(building_table, path[i]);
        Building* b2 = hashSearch(building_table, path[i + 1]);
        if (!b1 || !b2) continue;

        // ����������
        MoveToEx(hdc, b1->x, b1->y, NULL);
        LineTo(hdc, b2->x, b2->y);

        // �����ͷ����
        double dx = b2->x - b1->x;
        double dy = b2->y - b1->y;
        double length = sqrt(dx * dx + dy * dy);

        if (length > 0) {
            // ��ͷλ�ã�·���е㣩
            int mid_x = (b1->x + b2->x) / 2;
            int mid_y = (b1->y + b2->y) / 2;

            // ��λ��������
            double ux = dx / length;
            double uy = dy / length;

            // ��ͷ����
            const int arrow_size = 10;
            const int arrow_length = 15;

            // ��ͷ����
            int tip_x = mid_x + (int)(ux * arrow_length);
            int tip_y = mid_y + (int)(uy * arrow_length);

            // ��ͷ�ױ�����
            int base1_x = mid_x - (int)(uy * arrow_size);
            int base1_y = mid_y + (int)(ux * arrow_size);
            int base2_x = mid_x + (int)(uy * arrow_size);
            int base2_y = mid_y - (int)(ux * arrow_size);

            // ���Ƽ�ͷ
            POINT arrow[3] = { {tip_x, tip_y}, {base1_x, base1_y}, {base2_x, base2_y} };
            Polygon(hdc, arrow, 3);
        }
    }

    // 4. �����㣨��ɫ�����յ㣨��ɫ��
    Building* start = hashSearch(building_table, path[0]);
    Building* end = hashSearch(building_table, path[path_size - 1]);

    if (start) {
        HBRUSH startBrush = CreateSolidBrush(RGB(0, 255, 0));
        if (startBrush) {
            HGDIOBJ hOld = SelectObject(hdc, startBrush);
            Ellipse(hdc, start->x - 10, start->y - 10, start->x + 10, start->y + 10);
            SelectObject(hdc, hOld);
            DeleteObject(startBrush);
        }
    }

    if (end) {
        HBRUSH endBrush = CreateSolidBrush(RGB(255, 0, 0));
        if (endBrush) {
            HGDIOBJ hOld = SelectObject(hdc, endBrush);
            Ellipse(hdc, end->x - 10, end->y - 10, end->x + 10, end->y + 10);
            SelectObject(hdc, hOld);
            DeleteObject(endBrush);
        }
    }

    // 5. ������Դ
    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(pathPen);
    DeleteObject(arrowBrush);
}

/**
 * ��ʾ������Ϣ
 * @param hdc �豸�����ľ��
 * @param building_id ����ID
 */
 /**
  * ��ʾ������Ϣ
  * @param hdc �豸�����ľ��
  * @param building_id ����ID
  */
  /**
   * ��ʾ������Ϣ
   * @param hdc �豸�����ľ��
   * @param building_id ����ID
   */
void showBuildingInfo(HDC hdc, int building_id) {
    Building* b = hashSearch(building_table, building_id);
    if (b == NULL) return;

    // ������Ϣ�򱳾�
    HBRUSH infoBrush = CreateSolidBrush(RGB(255, 255, 255)); // ��ɫ
    HPEN infoPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));     // ��ɫ�߿�
    if (!infoBrush || !infoPen) return;

    HGDIOBJ hOldBrush = SelectObject(hdc, infoBrush);
    HGDIOBJ hOldPen = SelectObject(hdc, infoPen);
    Rectangle(hdc, 450, 0, 600, 120);  // ������Ϣ��

    // ��ʾ������Ϣ
    SetBkMode(hdc, TRANSPARENT);
    HFONT hInfoFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
        OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
        VARIABLE_PITCH, TEXT("΢���ź�"));
    if (hInfoFont) {
        HFONT hOldFont = (HFONT)SelectObject(hdc, hInfoFont);

        // ת����������Ϊ���ַ�
        wchar_t wname[50];
        wchar_t wdesc[100];
        MultiByteToWideChar(CP_ACP, 0, b->name, -1, wname, 50);
        MultiByteToWideChar(CP_ACP, 0, b->description, -1, wdesc, 100);

        // ʹ�ÿ��ַ����
        wchar_t info[200];
        swprintf_s(info, _countof(info), L"��������: %s", wname);
        TextOut(hdc, 460, 20, info, (int)wcslen(info));

        // ������������������ʾ��
        wcscpy_s(info, _countof(info), L"��������: ");
        wcsncat_s(info, _countof(info), wdesc, 10); // ��һ�����30�ַ�
        TextOut(hdc, 460, 50, info, (int)wcslen(info));

        // ���ʣ������������У�
        if (wcslen(wdesc) > 10) {
            wcscpy_s(info, _countof(info), L"          ");
            wcsncat_s(info, _countof(info), wdesc + 10, 70);
            TextOut(hdc, 460, 75, info, (int)wcslen(info));
        }

        swprintf_s(info, _countof(info), L"����λ��: (%d, %d)", b->x, b->y);
        TextOut(hdc, 460, 100, info, (int)wcslen(info));

        SelectObject(hdc, hOldFont);
        DeleteObject(hInfoFont);
    }

    // �ָ���ɾ��GDI����
    SelectObject(hdc, hOldBrush);
    SelectObject(hdc, hOldPen);
    DeleteObject(infoBrush);
    DeleteObject(infoPen);
}

/**
 * ���ڹ��̺���
 * �������д�����Ϣ
 */
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int start_id = -1;    // ��㽨��ID
    static int end_id = -1;      // �յ㽨��ID
    static POINT drag_start = { 0, 0 };  // �϶���ʼ��
    static int edit_edge_mode = 0;       // 0:�޲��� 1:��ӱ� 2:ɾ����
    static int first_building_id = -1;   // �߱༭�ĵ�һ������ID

    switch (msg) {
    case WM_CREATE:
        initializeCampusData();  // ��ʼ��У԰����
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // ���Ƶ�ͼ�ͽ���
        drawCampusMap(hdc);

        // ����·����������ڣ�
        if (shortest_path && path_size > 0) {
            drawPath(hdc, shortest_path, path_size);
        }

        // ��ʾ������Ϣ
        if (start_id != -1) {
            showBuildingInfo(hdc, start_id);
        }
        else if (end_id != -1) {
            showBuildingInfo(hdc, end_id);
        }

        // �༭ģʽ��ʾ
        if (is_edit_mode) {
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(255, 0, 0)); // ��ɫ����

            HFONT hFont = CreateFont(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS,
                CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("΢���ź�"));

            if (hFont) {
                HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
                TextOut(hdc, 20, 20, TEXT("�༭ģʽ (��E�˳�)"),
                    (int)_tcslen(TEXT("�༭ģʽ (��E�˳�)")));

                // ��ӱ�/ɾ����ģʽ��ʾ
                if (edit_edge_mode == 1) {
                    TextOut(hdc, 20, 50, TEXT("��ӱ�ģʽ: ѡ���һ��������A, ��ѡ��ڶ�������"),
                        (int)_tcslen(TEXT("��ӱ�ģʽ: ѡ���һ��������A, ��ѡ��ڶ�������")));
                }
                else if (edit_edge_mode == 2) {
                    TextOut(hdc, 20, 50, TEXT("ɾ����ģʽ: ѡ���һ��������D, ��ѡ��ڶ�������"),
                        (int)_tcslen(TEXT("ɾ����ģʽ: ѡ���һ��������D, ��ѡ��ڶ�������")));
                }

                SelectObject(hdc, hOldFont);
                DeleteObject(hFont);
            }
        }

        EndPaint(hwnd, &ps);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        // ��鰴ť���
        if (x >= 190 && x <= 300) {  // �ڰ�ť��ˮƽ��Χ��
            // �༭ģʽ��ť (Y:800-850)
            if (y >= 800 && y <= 850) {
                is_edit_mode = !is_edit_mode;
                selected_building_id = -1;
                edit_edge_mode = 0;
                first_building_id = -1;

                MessageBox(hwnd,
                    is_edit_mode ?
                    L"�ѽ���༭ģʽ��\n���ѡ�������Ҽ��϶�����λ��\nA:��ӱ� D:ɾ����" :
                    L"���˳��༭ģʽ��",
                    L"�༭ģʽ",
                    MB_OK | MB_ICONINFORMATION);

                InvalidateRect(hwnd, NULL, TRUE);  // �ػ洰��
                break;
            }
            // �˳�ϵͳ��ť (Y:830-880)
            else if (y >= 830 && y <= 880) {
                PostQuitMessage(0);
                break;
            }
        }

        if (is_edit_mode) {
            // �༭ģʽ��ѡ����
            for (int i = 0; i < MAX_BUILDINGS; i++) {
                // ������Ч����
                if (buildings[i].id == -1) continue;

                int dx = abs(x - buildings[i].x);
                int dy = abs(y - buildings[i].y);
                double distance = sqrt((double)(dx * dx + dy * dy));

                // �������ڽ�����Ƿ�Χ��
                if (distance <= 15) {
                    selected_building_id = i;

                    // �߱༭ģʽ����
                    if (edit_edge_mode != 0 && first_building_id == -1) {
                        first_building_id = selected_building_id;
                        // ��ʾ��ʾ��Ϣ
                        if (edit_edge_mode == 1) {
                            MessageBox(hwnd,
                                L"��ѡ��Ҫ���ӵĵڶ�����������������",
                                L"��ӱ�",
                                MB_OK | MB_ICONINFORMATION);
                        }
                        else if (edit_edge_mode == 2) {
                            MessageBox(hwnd,
                                L"��ѡ��Ҫ�Ͽ����ӵ�Ŀ�꽨������������",
                                L"ɾ����",
                                MB_OK | MB_ICONINFORMATION);
                        }
                    }
                    else if (edit_edge_mode != 0 && first_building_id != -1 &&
                        first_building_id != selected_building_id) {
                        // ��ӱ�ģʽ
                        if (edit_edge_mode == 1) {
                            // �����Ƿ��Ѵ���
                            int exists = 0;
                            if (campus_graph) { // ���ͼ��Ч�Լ��
                                for (int j = 0; j < campus_graph->E; j++) {
                                    if ((campus_graph->edges[j].src == first_building_id &&
                                        campus_graph->edges[j].dest == selected_building_id) ||
                                        (campus_graph->edges[j].src == selected_building_id &&
                                            campus_graph->edges[j].dest == first_building_id)) {
                                        exists = 1;
                                        break;
                                    }
                                }
                            }

                            if (!exists) {
                                // ������ͼ������һ���ߣ�
                                int new_edge_count = campus_graph ? campus_graph->E + 1 : 1;
                                Graph* new_graph = createGraph(MAX_BUILDINGS, new_edge_count);

                                if (!new_graph) {
                                    MessageBox(hwnd, L"������ͼʧ�ܣ�", L"����", MB_ICONERROR);
                                    break;
                                }

                                // ����ԭ�б�
                                if (campus_graph) {
                                    for (int j = 0; j < campus_graph->E; j++) {
                                        if (j < new_graph->E) {
                                            new_graph->edges[j] = campus_graph->edges[j];
                                        }
                                    }
                                }

                                // ����±�
                                new_graph->edges[new_edge_count - 1].src = first_building_id;
                                new_graph->edges[new_edge_count - 1].dest = selected_building_id;

                                // �ؽ��ڽӱ�
                                for (int j = 0; j < new_edge_count; j++) {
                                    Building* b1 = hashSearch(building_table, new_graph->edges[j].src);
                                    Building* b2 = hashSearch(building_table, new_graph->edges[j].dest);

                                    // ���NULL���
                                    if (!b1 || !b1) {
                                        continue; // ������Ч����
                                    }

                                    int dx = b1->x - b2->x;
                                    int dy = b1->y - b2->y;
                                    int weight = (int)sqrt(dx * dx + dy * dy);
                                    addEdge(new_graph, new_graph->edges[j].src,
                                        new_graph->edges[j].dest, weight, j);
                                }

                                // �滻��ͼ
                                Graph* old_graph = campus_graph;
                                campus_graph = new_graph;

                                // �ͷž�ͼ�ڴ棨������ڣ�
                                if (old_graph) {
                                    for (int j = 0; j < old_graph->V; j++) {
                                        AdjListNode* node = old_graph->array[j].head;
                                        while (node) {
                                            AdjListNode* temp = node;
                                            node = node->next;
                                            free(temp);
                                        }
                                    }
                                    free(old_graph->array);
                                    free(old_graph->edges);
                                    free(old_graph);
                                }

                                // ���������
                                saveEdgeData(campus_graph);

                                MessageBox(hwnd, L"����ӳɹ���", L"�ɹ�", MB_OK | MB_ICONINFORMATION);
                            }
                            else {
                                MessageBox(hwnd, L"�ñ��Ѵ��ڣ�", L"����", MB_ICONERROR);
                            }
                        }
                        // ɾ����ģʽ
                        else if (edit_edge_mode == 2) {
                            // ����Ҫɾ���ı�
                            int found_index = -1;
                            if (campus_graph) { // ���ͼ��Ч�Լ��
                                for (int j = 0; j < campus_graph->E; j++) {
                                    if ((campus_graph->edges[j].src == first_building_id &&
                                        campus_graph->edges[j].dest == selected_building_id) ||
                                        (campus_graph->edges[j].src == selected_building_id &&
                                            campus_graph->edges[j].dest == first_building_id)) {
                                        found_index = j;
                                        break;
                                    }
                                }
                            }

                            if (found_index != -1 && campus_graph) {
                                // ������ͼ������һ���ߣ�
                                int new_edge_count = campus_graph->E - 1;
                                Graph* new_graph = createGraph(MAX_BUILDINGS, new_edge_count);

                                if (!new_graph) {
                                    MessageBox(hwnd, L"������ͼʧ�ܣ�", L"����", MB_ICONERROR);
                                    break;
                                }

                                // ����δɾ���ı�
                                int new_index = 0;
                                for (int j = 0; j < campus_graph->E; j++) {
                                    if (j != found_index) {
                                        new_graph->edges[new_index++] = campus_graph->edges[j];
                                    }
                                }

                                // �ؽ��ڽӱ�
                                for (int j = 0; j < new_edge_count; j++) {
                                    Building* b1 = hashSearch(building_table, new_graph->edges[j].src);
                                    Building* b2 = hashSearch(building_table, new_graph->edges[j].dest);

                                    // ���NULL���
                                    if (!b1 || !b1) {
                                        continue; // ������Ч����
                                    }

                                    int dx = b1->x - b2->x;
                                    int dy = b1->y - b2->y;
                                    int weight = (int)sqrt(dx * dx + dy * dy);
                                    addEdge(new_graph, new_graph->edges[j].src,
                                        new_graph->edges[j].dest, weight, j);
                                }

                                // �滻��ͼ
                                Graph* old_graph = campus_graph;
                                campus_graph = new_graph;

                                // �ͷž�ͼ�ڴ�
                                for (int j = 0; j < old_graph->V; j++) {
                                    AdjListNode* node = old_graph->array[j].head;
                                    while (node) {
                                        AdjListNode* temp = node;
                                        node = node->next;
                                        free(temp);
                                    }
                                }
                                free(old_graph->array);
                                free(old_graph->edges);
                                free(old_graph);

                                // ���������
                                saveEdgeData(campus_graph);

                                MessageBox(hwnd, L"��ɾ���ɹ���", L"�ɹ�", MB_OK | MB_ICONINFORMATION);
                            }
                            else {
                                MessageBox(hwnd, L"δ�ҵ��ñߣ�", L"����", MB_ICONERROR);
                            }
                        }

                        // ���ñ༭״̬
                        edit_edge_mode = 0;
                        first_building_id = -1;
                    }

                    InvalidateRect(hwnd, NULL, TRUE);  // �ػ�
                    break;
                }
            }
        }
        else {
            // ��ͨģʽ���������
            for (int i = 0; i < MAX_BUILDINGS; i++) {
                // ������Ч����
                if (buildings[i].id == -1) continue;

                int dx = abs(x - buildings[i].x);
                int dy = abs(y - buildings[i].y);
                double distance = sqrt((double)(dx * dx + dy * dy));

                if (distance <= 15) {
                    start_id = i;

                    // ����������յ㣬����·�������ͼ��Ч�Լ�飩
                    if (start_id != -1 && end_id != -1 && campus_graph) {
                        int dist[MAX_BUILDINGS];
                        int prev[MAX_BUILDINGS];

                        // ȷ��ͼ��������Ч
                        if (start_id < campus_graph->V && end_id < campus_graph->V) {
                            dijkstra(campus_graph, start_id, dist, prev);

                            if (shortest_path) {
                                free(shortest_path);
                                shortest_path = NULL;
                            }
                            shortest_path = getShortestPath(prev, end_id, &path_size);
                            if (!shortest_path) path_size = 0;
                        }
                    }

                    InvalidateRect(hwnd, NULL, TRUE);  // �ػ�
                    break;
                }
            }
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if (is_edit_mode) {
            // �༭ģʽ����ʼ�϶�����
            for (int i = 0; i < MAX_BUILDINGS; i++) {
                // ������Ч����
                if (buildings[i].id == -1) continue;

                int dx = abs(x - buildings[i].x);
                int dy = abs(y - buildings[i].y);
                double distance = sqrt((double)(dx * dx + dy * dy));

                if (distance <= 15) {
                    selected_building_id = i;
                    drag_start.x = x;
                    drag_start.y = y;
                    SetCapture(hwnd); // �������
                    break;
                }
            }
        }
        else {
            // ��ͨģʽ�������յ�
            for (int i = 0; i < MAX_BUILDINGS; i++) {
                // ������Ч����
                if (buildings[i].id == -1) continue;

                int dx = abs(x - buildings[i].x);
                int dy = abs(y - buildings[i].y);
                double distance = sqrt((double)(dx * dx + dy * dy));

                if (distance <= 15) {
                    end_id = i;

                    // ����������յ㣬����·�������ͼ��Ч�Լ�飩
                    if (start_id != -1 && end_id != -1 && campus_graph) {
                        int dist[MAX_BUILDINGS];
                        int prev[MAX_BUILDINGS];

                        // ȷ��ͼ��������Ч
                        if (start_id < campus_graph->V && end_id < campus_graph->V) {
                            dijkstra(campus_graph, start_id, dist, prev);

                            if (shortest_path) {
                                free(shortest_path);
                                shortest_path = NULL;
                            }
                            shortest_path = getShortestPath(prev, end_id, &path_size);
                            if (!shortest_path) path_size = 0;
                        }
                    }

                    InvalidateRect(hwnd, NULL, TRUE);  // �ػ�
                    break;
                }
            }
        }
        break;
    }

    case WM_MOUSEMOVE: {
        // �༭ģʽ���϶�����
        if (is_edit_mode && selected_building_id != -1 && (wParam & MK_RBUTTON)) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            // �����ƶ�����
            int dx = x - drag_start.x;
            int dy = y - drag_start.y;

            // ���½�������
            buildings[selected_building_id].x += dx;
            buildings[selected_building_id].y += dy;

            // ������ʼ��
            drag_start.x = x;
            drag_start.y = y;

            // ���¹�ϣ���еĽ�����Ϣ
            Building* b = &buildings[selected_building_id];
            HashNode* node = building_table->table[hash(b->id)];
            while (node != NULL && node->key != b->id) {
                node = node->next;
            }
            if (node) node->building = *b;

            // ����������رߵ�Ȩ�أ����ͼ��Ч�Լ�飩
            if (campus_graph) {
                updateEdgeWeight(campus_graph, buildings[selected_building_id].id);
            }

            InvalidateRect(hwnd, NULL, TRUE);  // �ػ�
        }
        break;
    }

    case WM_RBUTTONUP: {
        // �����϶�
        if (is_edit_mode && selected_building_id != -1) {
            selected_building_id = -1;
            ReleaseCapture(); // �ͷ���겶��

            // �����޸ĺ�Ľ�������
            saveBuildingData();
        }
        break;
    }

    case WM_KEYDOWN:
        if (wParam == 'E' || wParam == 'e') {  // �л��༭ģʽ
            is_edit_mode = !is_edit_mode;
            selected_building_id = -1;
            edit_edge_mode = 0;
            first_building_id = -1;

            MessageBox(hwnd,
                is_edit_mode ?
                L"�ѽ���༭ģʽ��\n���ѡ�������Ҽ��϶�����λ��\nA:��ӱ� D:ɾ����" :
                L"���˳��༭ģʽ��",
                L"�༭ģʽ",
                MB_OK | MB_ICONINFORMATION);

            InvalidateRect(hwnd, NULL, TRUE);  // �ػ�
        }
        else if (wParam == VK_SPACE) {  // ���·��
            start_id = end_id = -1;
            if (shortest_path) {
                free(shortest_path);
                shortest_path = NULL;
            }
            path_size = 0;
            InvalidateRect(hwnd, NULL, TRUE);  // �ػ�
        }
        else if (wParam == VK_ESCAPE) {  // �˳�����
            PostQuitMessage(0);
        }
        else if (is_edit_mode) {
            // ��ӱ�ģʽ
            if (wParam == 'A' || wParam == 'a') {
                if (selected_building_id != -1) {
                    edit_edge_mode = 1;
                    first_building_id = selected_building_id;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
            // ɾ����ģʽ
            else if (wParam == 'D' || wParam == 'd') {
                if (selected_building_id != -1) {
                    edit_edge_mode = 2;
                    first_building_id = selected_building_id;
                    InvalidateRect(hwnd, NULL, TRUE);
                }
            }
        }
        break;

    case WM_DESTROY:
        // �˳�ǰ��������
        saveBuildingData();
        if (campus_graph) saveEdgeData(campus_graph);  // ���������
        PostQuitMessage(0);  // �˳���Ϣѭ��
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam); // Ĭ����Ϣ����
    }
    return 0;
}

/**
 * ����ȫ����Դ
 */
void cleanupResources() {
    // �ͷ�ͼ�ṹ
    if (campus_graph) {
        // �ͷ������ڽӱ�ڵ�
        for (int i = 0; i < campus_graph->V; i++) {
            AdjListNode* node = campus_graph->array[i].head;
            while (node) {
                AdjListNode* temp = node;
                node = node->next;
                free(temp);
            }
        }
        // �ͷ�ͼ�ṹ�ڴ�
        free(campus_graph->array);
        free(campus_graph->edges);
        free(campus_graph);
        campus_graph = NULL;
    }

    // �ͷŹ�ϣ��
    if (building_table) {
        for (int i = 0; i < HASH_SIZE; i++) {
            HashNode* node = building_table->table[i];
            while (node) {
                HashNode* temp = node;
                node = node->next;
                free(temp);
            }
        }
        free(building_table->table);
        free(building_table);
        building_table = NULL;
    }

    // �ͷ����·��
    if (shortest_path) {
        free(shortest_path);
        shortest_path = NULL;
        path_size = 0;
    }

    // �ͷ�λͼ
    if (campus_map) {
        DeleteObject(campus_map);
        campus_map = NULL;
    }
}

/**
 * ������
 * ������ڵ�
 */
int main() {
    // ��ʼ��ȫ��ָ�루��Ȼ������Ϊȫ�֣���ȷ����ʼ״̬��
    campus_graph = NULL;
    building_table = NULL;
    shortest_path = NULL;
    campus_map = NULL;
    path_size = 0;
    is_edit_mode = false;
    selected_building_id = -1;

    // ��ȡ��ǰʵ�����
    HINSTANCE hInstance = GetModuleHandle(NULL);
    int nCmdShow = SW_SHOW;

    // ע�ᴰ����
    const wchar_t CLASS_NAME[] = L"CampusGuideWindowClass";

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;      // ���ڹ��̺���
    wc.hInstance = hInstance;      // ��ǰʵ��
    wc.lpszClassName = CLASS_NAME; // ����
    wc.hCursor = LoadCursor(NULL, IDC_ARROW); // �����ʽ
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // ����ɫ

    if (!RegisterClass(&wc)) {
        MessageBox(NULL, L"����ע��ʧ��!", L"����", MB_ICONERROR);
        return 1;
    }

    // ����������
    hwnd = CreateWindow(
        CLASS_NAME,
        L"У԰����ϵͳ",            // ���ڱ���
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, // ������ʽ
        CW_USEDEFAULT, CW_USEDEFAULT, // λ��
        WINDOW_WIDTH + 16, WINDOW_HEIGHT + 39, // �ߴ磨���Ǳ߿�
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd) {
        MessageBox(NULL, L"���ڴ���ʧ��!", L"����", MB_ICONERROR);
        return 1;
    }

    // ��ʾ����
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    // ��Ϣѭ��
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ������Դ
    cleanupResources();

    return (int)msg.wParam;
}