#pragma once
#include <vector>
#include <cmath>
#include <SFML/Graphics.hpp>

struct VerletObject {
    sf::Vector2f position;
    sf::Vector2f position_last;
    sf::Vector2f acceleration;
    float radius = 10.0f;
    sf::Color color = sf::Color::White;


    VerletObject() = default;

    VerletObject(sf::Vector2f position_, float radius_)
        : position{position_}
        , position_last{position_}
        , acceleration{0.0f,0.0f}
        , radius{radius_}
        {}


    void update(float dt) {
        // Updates the postiion of the verletObject based off previous position and acceleration
        sf::Vector2f displacement = position - position_last;
        position_last = position;
        position = position + displacement + acceleration * dt * dt;
        acceleration = {};
    }
    
    // Sets the velocity by changing the last position
    void setVelocity(sf::Vector2f v, float dt) {
        position_last = position - (v * dt);
    }

    void accelerate(sf::Vector2f acc) {
        acceleration += acc;
    }


};


class Solver {
    public:
        Solver() = default;

        const sf::Vector2f gravity{0.0f,1000.0f};

        void update() {
            // Updates the solver, which updates every verletObject to reflect gravity, collisions, and constraints.
            m_time += m_frame_dt;
            const float step_dt = getStepDt();
            for (uint32_t i{m_sub_steps}; i--;) {
                applyGravity();
                checkCollisions(step_dt);
                applyConstraints();
                updateObjects(step_dt);
            }
        }

        void setSimulationUpdateRate(uint32_t rate) {
            m_frame_dt = 1.0f / static_cast<float>(rate);
        }

        void setConstraint(sf::Vector2f position, float radius) {
            m_constraint_center = position;
            m_constraint_radius = radius;
        }

        void setSubStepsCount(uint32_t sub_steps) {
            m_sub_steps = sub_steps;
        }

        void setObjectVelocity(VerletObject& object, sf::Vector2f v) {
            object.setVelocity(v, getStepDt());
        }

        [[nodiscard]]
        const std::vector<VerletObject>& getObjects() const {
            return m_objects;
        }

        [[nodiscard]]
        sf::Vector3f getConstraint() const {
            return {m_constraint_center.x, m_constraint_center.y, m_constraint_radius};
        }

        [[nodiscard]]
        uint64_t getObjectsCount() const {
            return m_objects.size();
        }

        [[nodiscard]]
        float getTime() const {
            return m_time;
        }

        [[nodiscard]]
        float getStepDt() const {
            return m_frame_dt / static_cast<float>(m_sub_steps);
        }


        VerletObject& addObjects(sf::Vector2f position, float radius) {
            return m_objects.emplace_back(position, radius);
        }

        void applyGravity() {
            for (auto& v: m_objects) {
                v.accelerate(gravity);
            }
        }

        void updateObjects(float dt) {
            for (auto& v: m_objects) {
                v.update(dt);
            }
        }

    private:
        // Solver attributes
        uint32_t m_sub_steps = 1;
        sf::Vector2f m_gravity = {0.0f, 1000.0f};
        sf::Vector2f m_constraint_center;
        float m_constraint_radius  = 100.0f;
        std::vector<VerletObject> m_objects;
        float m_time = 0.0f;
        float m_frame_dt = 0.0f;

        void applyConstraints() {
            // Loops through objects to check if they violate a constraint. This is done by finding the distance between the center
            // of the object and the constraint center. If this distance is greater than the constraint radius subtracted by the
            // object radius that means the edge of the object is outside the constraint. 

            // To adjust for this we find the unit vector in the direction towards the constraint center and then shift the object 
            // in that direction
            for (auto& obj: m_objects) {
                const sf::Vector2f vec = m_constraint_center - obj.position;
                const float dist = sqrt(vec.x * vec.x + vec.y * vec.y);
                if (dist > (m_constraint_radius - obj.radius)) {
                    const sf::Vector2f n = vec / dist;
                    obj.position = m_constraint_center - n * (m_constraint_radius - obj.radius);
                }
            }
        }

        void checkCollisions(float dt) {
            // Checks collisions by checking every combination of objects and seeing if the distance between the 
            // first object and second object is less than the combined radius. If it is then there is a collision. 

            // To adjust for this we find the collision axis and then shift the position of the objects along that axis. 
            // The response coefficent is there to soften the collisions. Then the two objects are shifted based off their respective
            // masses -- the object with more mass will move less and vice versa.
            const float response_coefficient = 0.75f;
            const int64_t objects_count = m_objects.size();
            for (int64_t i{0}; i < objects_count; i++) {
                VerletObject& firstObj = m_objects[i];
                for (int64_t k{i+1}; k < objects_count; k++) {
                    VerletObject& secondObj = m_objects[k];
                    sf::Vector2f vec = firstObj.position - secondObj.position;
                    float dist = sqrt(vec.x * vec.x + vec.y * vec.y);
                    float combined_radius = firstObj.radius + secondObj.radius;
                    if (dist < combined_radius) {
                        sf::Vector2f direction = vec / dist;
                        float first_ratio = firstObj.radius / combined_radius;
                        float second_ratio = secondObj.radius / combined_radius;
                        float delta = 0.5f * response_coefficient * (dist - combined_radius);

                        firstObj.position -= direction * second_ratio * delta;
                        secondObj.position += direction * first_ratio * delta;
                    }
                }
            }
        }
};
