#include <iostream>
#include <vector>
#include <chrono>
#include <string>

const float DT = 0.01f;
const float MIN_DISTANCE = 0.01f;

struct body {

    void update(float dt) {
        pos[0] += vel[0] * dt; pos[1] += vel[1] * dt;
        vel[0] += acc[0] * dt; vel[1] += acc[1] * dt;
        acc[0] = 0.0f; acc[1] = 0.0f;
    }

    float pos[2];
    float vel[2];
    float acc[2];
    float mass;
};

struct simulation {

    simulation() {}

    simulation(size_t size, float seed) {
        srand(seed);

        bodies = std::vector<body>(size);

        for (size_t i = 0; i < size; i++) {
            float x = rand() % size;
            float y = rand() % size;

            float d_x = rand() % size;
            float d_y = rand() % size;

            bodies[i].pos[0] = x; bodies[i].pos[1] = y;
            bodies[i].vel[0] = d_x; bodies[i].vel[1] = d_y;
            bodies[i].mass = 1.0f;
        }
    }

    void update() {

        #pragma omp parallel for
        for (size_t i = 0; i < bodies.size(); i++) {
            float p1[2] = { bodies[i].pos[0], bodies[i].pos[1] };
            float m1 = bodies[i].mass;

            for (size_t j = i + 1; j < bodies.size(); j++) {
                float p2[2] = { bodies[j].pos[0], bodies[j].pos[1] };
                float m2 = bodies[j].mass;

                float r[2] = { p2[0] - p1[0], p2[1] - p1[1] };
                float mag_sq = (r[0] * r[0]) + (r[1] * r[1]);
                float temp = std::max(mag_sq, MIN_DISTANCE) * std::sqrt(mag_sq);

                float d_acc[2] = { r[0] / temp, r[1] / temp };

                bodies[i].acc[0] += m2 * d_acc[0]; bodies[i].acc[1] += m2 * d_acc[1];
                bodies[j].acc[0] -= m1 * d_acc[0]; bodies[j].acc[1] -= m1 * d_acc[1];
            }
        }

        for (size_t i = 0; i < bodies.size(); i++) {
            bodies[i].update(DT);
        }
    }

    std::vector<body> bodies;
};

simulation sim;
float* data;

extern "C" void initialize(int n, float seed) {
    sim = simulation(n, seed);

    data = (float*)malloc(n * 3 * sizeof(float));
}

extern "C" void update() {
    sim.update();
}

extern "C" float* get_bodies() {

    for (int i = 0; i < sim.bodies.size(); i++) {
        data[i] = sim.bodies[i].pos[0];
        data[i + 1] = sim.bodies[i].pos[1];
        data[i + 2] = sim.bodies[i].mass;
    }

    return &data[0];
}