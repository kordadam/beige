#include "Game.hpp"

#include <core/Logger.hpp>

#include <algorithm>
#include <string>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

Game::Game() :
IGame(
    {
        100u, 100u, 1280u, 720u, "Beige Testbed"
    }
),
m_input { bc::Input::getInstance() } {
    bc::Logger::info("Game object has been created!");

    m_state.cameraPosition = glm::vec3(0.0f, 0.0f, 30.0f);
    m_state.cameraEuler = glm::vec3(0.0f, glm::pi<float>(), 0.0f);

    m_state.cameraViewDirty = true;
}

Game::~Game() {
    bc::Logger::info("Game object has been destroyed!");
}

auto Game::update(const float deltaTime) -> bool {
    // HACK: Temporary hack to move camera.
    if (m_input->isKeyDown(bc::Key::A) || m_input->isKeyDown(bc::Key::Left)) {
        cameraYaw(1.0f * deltaTime);
    }

    if (m_input->isKeyDown(bc::Key::D) || m_input->isKeyDown(bc::Key::Right)) {
        cameraYaw(-1.0f * deltaTime);
    }

    if (m_input->isKeyDown(bc::Key::Up)) {
        cameraPitch(1.0f * deltaTime);
    }

    if (m_input->isKeyDown(bc::Key::Down)) {
        cameraPitch(-1.0f * deltaTime);
    }

    const float temporaryMovementSpeed { 50.0f };
    glm::vec3 velocity { 0.0f };

    if (m_input->isKeyDown(bc::Key::W)) {
        const glm::vec3 forward { m_state.cameraLook };
        velocity += forward;
    }

    if (m_input->isKeyDown(bc::Key::S)) {
        const glm::vec3 backward { -m_state.cameraLook };
        velocity += backward;
    }

    if (m_input->isKeyDown(bc::Key::Q)) {
        const glm::vec3 left { -m_state.cameraRight };
        velocity += left;
    }

    if (m_input->isKeyDown(bc::Key::E)) {
        const glm::vec3 right { m_state.cameraRight };
        velocity += right;
    }

    if (m_input->isKeyDown(bc::Key::Space)) {
        const glm::vec3 up { m_state.cameraUp };
        velocity += up;
    }

    if (m_input->isKeyDown(bc::Key::X)) {
        const glm::vec3 down { -m_state.cameraUp };
        velocity += down;
    }

    if (velocity != glm::vec3(0.0f)) {
        m_state.cameraPosition += velocity * temporaryMovementSpeed * deltaTime;
        m_state.cameraViewDirty = true;
    }

    recalculateView();

    return true;
}

auto Game::render(const float deltaTime) -> bool {
    return true;
}

auto Game::onResize(const uint32_t width, const uint32_t height) -> void {

}

auto Game::recalculateView() -> void {
    if (m_state.cameraViewDirty) {
        const glm::vec3 newLook {
             std::cos(m_state.cameraEuler.x) * std::sin(m_state.cameraEuler.y),
             std::sin(m_state.cameraEuler.x),
             std::cos(m_state.cameraEuler.x) * std::cos(m_state.cameraEuler.y)
        };

        m_state.cameraLook = glm::normalize(newLook);
        m_state.cameraRight = glm::normalize(glm::cross(m_state.cameraLook, glm::vec3(0.0f, 1.0f, 0.0f)));
        m_state.cameraUp = glm::normalize(glm::cross(m_state.cameraRight, m_state.cameraLook));

        m_state.view = glm::lookAt(m_state.cameraPosition, m_state.cameraPosition + m_state.cameraLook, glm::vec3(0.0f, 1.0f, 0.0f));

        m_state.cameraViewDirty = false;
    }
}

auto Game::cameraYaw(const float amount) -> void {
    m_state.cameraEuler.y += amount;
    m_state.cameraViewDirty = true;
}

auto Game::cameraPitch(const float amount) -> void {
    m_state.cameraEuler.x += amount;

    // Clamp to avoid gimball lock.
    const float limit { glm::radians(89.0f) };
    m_state.cameraEuler.x = std::clamp<float>(m_state.cameraEuler.x, -limit, limit);

    m_state.cameraViewDirty = true;
}

auto Game::cameraRoll(const float amount) -> void {
    m_state.cameraEuler.z += amount;
    m_state.cameraViewDirty = true;
}