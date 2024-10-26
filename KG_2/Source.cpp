#include <GL/freeglut.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <Windows.h>

using namespace std;

const int WIDTH = 512;
const int HEIGHT = 512;

float WHITE[3] = { 1, 1, 1 };

//Структура для представления точки в 3D пространстве
struct Point3D {
    float x, y, z;
};

//Структура для представления многоугольника с цветом
struct MyPolygon {
    vector<Point3D> vertices;
    float color[3];
};

//Массив многоугольников
vector<MyPolygon> polygons;
float zBuffer[WIDTH][HEIGHT];
float frameBuffer[WIDTH][HEIGHT][3];

//Функция выбора набора многоугольников: фиксированные или случайные
void selectPolygons() {
    cout << "Выберите режим:\n1. Пример с последней страницы лекции\n2. Непересекающиеся 4 многоугольника" << endl;
    cout << "3. 5 треугольников, каждый предыдущий протыкает текущий\n4. Шестиугольник протыкается треугольниками" << endl << endl;
    int choice;
    cin >> choice;

    polygons.clear();

    switch (choice) {
    case 1: {
        polygons = {
            { {{100, 30, 20}, {100, 280, 20}, {220, 280, 20}, {220, 30, 20}}, {0, 0, 1} },
            { {{50, 120, 10}, {50, 200, 10}, {270, 200, 10}, {270, 120, 10}}, {0, 1, 0} },
            { {{150, 150, 25}, {250, 250, 5}, {300, 100, 5}}, {1, 0, 0} }
        };
        break;
    }
    case 2: {
        polygons = {
            { {{50, 50, 20}, {50, 150, 5}, {150, 100, 10}}, {0, 0, 1} }, //Синий треугольник
            { {{100, 250, 30}, {200, 250, 30}, {200, 350, 30}, {100, 350, 30}}, {0, 1, 0} }, //Зеленый квадрат
            { {{350, 100, 10}, {400, 150, 10}, {450, 120, 10}, {420, 50, 10}, {360, 30, 10}}, {1, 0, 0} }, //Красный пятиугольник
            { {{300, 300, 10}, {400, 400, 20}, {450, 350, 30}, {450, 250, 40}, {350, 250, 50}, {250, 300, 60}}, {1, 0.5, 0} }  //Оранжевый шестиугольник
        };
        break;
    }
    case 3: {
        polygons = {
        { {{90, 100, 40}, {140, 200, 50}, {40, 200, 40}}, {0, 0, 1} }, //Треугольник 1 (синий)
        { {{120, 120, 45}, {170, 220, 40}, {70, 220, 45}}, {0, 1, 0} }, //Треугольник 2 (зеленый)
        { {{150, 140, 40}, {200, 240, 50}, {100, 240, 40}}, {1, 0, 0} }, //Треугольник 3 (красный)
        { {{180, 160, 45}, {230, 260, 40}, {130, 260, 45}}, {1, 0.5, 0} }, //Треугольник 4 (оранжевый)
        { {{210, 180, 40}, {260, 280, 50}, {160, 280, 40}}, {1, 1, 0} } //Треугольник 5 (желтый)
        };
        break;
    }
    case 4: {
        polygons = {
            //Шестиугольник
            {{  { 256, 200, 20 }, { 356, 235, 20 }, { 356, 345, 20 },
                { 256, 380, 20 }, { 156, 345, 20 }, { 156, 235, 20 }}
                , { 0, 0, 1 } //Синий
            },
            //Треугольник 1
            {{{ 356, 200, 10 }, { 356, 345, 10 }, { 306, 200, 40 }}, { 0, 1, 0 } //Зеленый
            },
            //Треугольник 2
            {{{ 170, 235, 40 }, { 170, 345, 40 }, { 306, 200, 10 }}, { 1, 0.5, 0 } //Оранжевый
            },
            //Треугольник 3
            {{{ 306, 200, 40 }, { 156, 235, 10 }, { 306, 380, 10 }}, { 1, 1, 0 } //Желтый
            },
            //Треугольник 4
            {{{ 200, 345, 40 }, { 306, 380, 40 }, { 356, 345, 10 }}, { 1, 0, 0 } //Красный
            }
        };
        break;
    }
    default: {
        throw runtime_error("Неверный выбор"); return;
    }
    }

    //Вывод информации о многоугольниках
    cout << "Выбранные многоугольники:\n";
    for (int i = 0; i < polygons.size(); i++) {
        const auto& poly = polygons[i];
        cout << "Многоугольник " << (i + 1) << endl;
        for (const auto& vertex : poly.vertices) {
            cout << "(" << vertex.x << ", " << vertex.y << ", " << vertex.z << ")\t";
        }
        cout << endl;
    }
    cout << endl;
}

//Инициализация буферов
void initializeBuffers() {
    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            zBuffer[x][y] = -1;
            frameBuffer[x][y][0] = WHITE[0];
            frameBuffer[x][y][1] = WHITE[1];
            frameBuffer[x][y][2] = WHITE[2];
        }
    }
}

void drawPixel(int x, int y, const float color[3]) {
    glColor3fv(color);
    glBegin(GL_POINTS);
    glVertex2i(x, y);
    glEnd();
}

//Функция для вычисления коэффициентов уравнения плоскости
void calculatePlaneCoefficients(const MyPolygon& poly, float& A, float& B, float& C, float& D) {
    Point3D v0 = poly.vertices[0];
    Point3D v1 = poly.vertices[1];
    Point3D v2 = poly.vertices[2];

    A = (v1.y - v0.y) * (v2.z - v0.z) - (v1.z - v0.z) * (v2.y - v0.y);
    B = (v1.z - v0.z) * (v2.x - v0.x) - (v1.x - v0.x) * (v2.z - v0.z);
    C = (v1.x - v0.x) * (v2.y - v0.y) - (v1.y - v0.y) * (v2.x - v0.x);
    D = -(A * v0.x + B * v0.y + C * v0.z);
}

//Проверка, находится ли точка (x, y) внутри многоугольника произвольной формы
bool isPointInPolygon(int x, int y, const MyPolygon& poly) {
    int intersections = 0;
    int numVertices = poly.vertices.size();

    for (int i = 0; i < numVertices; i++) {
        Point3D v1 = poly.vertices[i];
        Point3D v2 = poly.vertices[(i + 1) % numVertices];

        if (((v1.y > y) != (v2.y > y)) &&
            (x < (v2.x - v1.x) * (y - v1.y) / (v2.y - v1.y) + v1.x)) {
            intersections++;
        }
    }

    return intersections % 2 == 1; //Точка внутри, если количество пересечений нечетное
}

//Функция заполнения Z-буфера для многоугольника
void fillPolygonZBuffer(const MyPolygon& poly) {
    float A, B, C, D;
    calculatePlaneCoefficients(poly, A, B, C, D);

    int minX = WIDTH, maxX = 0, minY = HEIGHT, maxY = 0;
    for (const auto& vertex : poly.vertices) {
        int x = static_cast<int>(vertex.x);
        int y = static_cast<int>(vertex.y);
        minX = min(minX, x);
        maxX = max(maxX, x);
        minY = min(minY, y);
        maxY = max(maxY, y);
    }

    minX = max(minX, 0);
    maxX = min(maxX, WIDTH - 1);
    minY = max(minY, 0);
    maxY = min(maxY, HEIGHT - 1);

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            if (!isPointInPolygon(x, y, poly)) {
                continue;
            }
            float z = -(A * x + B * y + D) / C;
            if (z > zBuffer[x][y]) {
                zBuffer[x][y] = z;
                copy(poly.color, poly.color + 3, frameBuffer[x][y]);
                drawPixel(x, y, poly.color);
                glutSwapBuffers();
            }
        }
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    glOrtho(0, WIDTH, 0, HEIGHT, -1, 1);

    for (int x = 0; x < WIDTH; x++) {
        for (int y = 0; y < HEIGHT; y++) {
            drawPixel(x, y, frameBuffer[x][y]);
        }
    }
    glutSwapBuffers();
}

void handleKeypress(unsigned char key, int x, int y) {
    if (key == 'z' || key == 'Z') {
        initializeBuffers();
        for (const auto& poly : polygons) {
            fillPolygonZBuffer(poly);
        }
        display();
    }
}

void initializeGL() {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glPointSize(1);
}

int main(int argc, char** argv) {
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    try
    {
        selectPolygons();

        glutInit(&argc, argv);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(WIDTH, HEIGHT);
        glutCreateWindow("Z-Buffer");

        initializeGL();

        glutDisplayFunc(display);
        glutKeyboardFunc(handleKeypress);

        glutMainLoop();
        return 0;
    }
    catch (const runtime_error& e) {
        cerr << e.what() << endl;
    }
}
