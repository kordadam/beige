#include "Game.hpp"

#include <core/Logger.hpp>
#include <math/Math.hpp>

#include <algorithm>
#include <string>
#include <iostream>

Game::Game() :
IGame(
    {
        100u, 100u, 1280u, 720u, "Beige Testbed"
    }
),
m_input { bc::Input::getInstance() } {
    bc::Logger::info("Game object has been created!");

    m_state.cameraPosition = bm::Vector3 { 0.0f, 0.0f, 30.0f };
    m_state.cameraEuler = bm::Vector3::zero();
    m_state.view = bm::Matrix4x4 {
        bm::Matrix4x4::translation(m_state.cameraPosition)
    };
    m_state.view.inverse();
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
    bm::Vector3 velocity { bm::Vector3::zero() };

    if (m_input->isKeyDown(bc::Key::W)) {
        const bm::Vector3 forward { m_state.view.forward() };
        velocity = velocity + forward;
    }

    if (m_input->isKeyDown(bc::Key::S)) {
        const bm::Vector3 backward { m_state.view.backward() };
        velocity = velocity + backward;
    }

    if (m_input->isKeyDown(bc::Key::Q)) {
        const bm::Vector3 left { m_state.view.left() };
        velocity = velocity + left;
    }

    if (m_input->isKeyDown(bc::Key::E)) {
        const bm::Vector3 right { m_state.view.right() };
        velocity = velocity + right;
    }

    if (m_input->isKeyDown(bc::Key::Space)) {
        velocity.y += 1.0f;
    }

    if (m_input->isKeyDown(bc::Key::X)) {
        velocity.y -= 1.0f;
    }

    if (
        std::abs(velocity.x) > 0.0002f ||
        std::abs(velocity.y) > 0.0002f ||
        std::abs(velocity.z) > 0.0002f
    ) {
        velocity.normalize();
        m_state.cameraPosition.x += velocity.x * temporaryMovementSpeed * deltaTime;
        m_state.cameraPosition.y += velocity.y * temporaryMovementSpeed * deltaTime;
        m_state.cameraPosition.z += velocity.z * temporaryMovementSpeed * deltaTime;
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
        const bm::Matrix4x4 rotation {
            bm::Matrix4x4::xyzEuler(
                m_state.cameraEuler.x,
                m_state.cameraEuler.y,
                m_state.cameraEuler.z
            )
        };
        const bm::Matrix4x4 translation {
            bm::Matrix4x4::translation(m_state.cameraPosition)
        };

        m_state.view = rotation * translation;
        m_state.view.inverse();
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
    const float limit { bm::Quaternion::degToRad(89.0f) };
    m_state.cameraEuler.x = std::clamp<float>(m_state.cameraEuler.x, -limit, limit);

    m_state.cameraViewDirty = true;
}

auto Game::cameraRoll(const float amount) -> void {
    m_state.cameraEuler.z += amount;
    m_state.cameraViewDirty = true;
}