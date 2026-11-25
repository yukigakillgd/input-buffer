#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

using namespace geode::prelude;

// Input buffer
//
// Vanilla GD only activates an orb (or a landing jump) if the click happens on
// the exact physics step the player is touching it. This mod remembers a jump
// press that arrived while the player could not act on it, and re-issues that
// press on every physics step inside a short window for as long as the button
// is still physically held. The first step the player touches an orb or lands,
// the press goes through.
//
// Everything is counted in physics steps (not frames, not wall time), so the
// result is deterministic and independent of FPS.

namespace {
    constexpr int kJump = 1; // PlayerButton::Jump

    struct Slot {
        bool held = false;        // button is physically down
        bool armed = false;       // an unconsumed early press is waiting
        int64_t pressStep = 0;    // step on which that press arrived
    };
}

class $modify(IBGameLayer, GJBaseGameLayer) {
    struct Fields {
        int64_t step = 0;
        Slot slots[2];            // [0] = player 1, [1] = player 2
    };

    PlayerObject* playerFor(int i) {
        return i == 0 ? m_player1 : m_player2;
    }

    // Modes where a press is a continuous hold or a per-click impulse that must
    // not be re-fired: ship, UFO, wave, swing copter.
    static bool isHoldMode(PlayerObject* p) {
        return p->m_isShip || p->m_isBird || p->m_isDart || p->m_isSwing;
    }

    void handleButton(bool down, int button, bool isPlayer1) {
        if (button == kJump && Mod::get()->getSettingValue<bool>("enabled")) {
            auto& s = m_fields->slots[isPlayer1 ? 0 : 1];
            if (down) {
                auto* p = playerFor(isPlayer1 ? 0 : 1);
                s.held = true;
                // If the player is already on the ground the game handles this
                // press itself. Only airborne presses need buffering.
                s.armed = p && !p->m_isOnGround;
                s.pressStep = m_fields->step;
            } else {
                s.held = false;
                s.armed = false;
            }
        }
        GJBaseGameLayer::handleButton(down, button, isPlayer1);
    }

    void processCommands(float dt) {
        GJBaseGameLayer::processCommands(dt);

        m_fields->step++;

        if (!Mod::get()->getSettingValue<bool>("enabled")) return;
        auto window = Mod::get()->getSettingValue<int64_t>("window-steps");

        for (int i = 0; i < 2; ++i) {
            auto& s = m_fields->slots[i];
            if (!s.armed) continue;

            auto* p = playerFor(i);
            if (!s.held || !p || p->m_isDead || isHoldMode(p) ||
                m_fields->step - s.pressStep > window) {
                s.armed = false;
                continue;
            }

            p->pushButton(PlayerButton::Jump);

            // Landing consumes the buffered press. In the air it stays armed
            // for the rest of the window so an orb touched later still fires.
            if (p->m_isOnGround) s.armed = false;
        }
    }
};
