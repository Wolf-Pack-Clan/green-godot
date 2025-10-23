#include "modules/efx/effect_def.h"

void EffectDefinition::add_particle_component(const ParticleComponent &pc) {
    print_line("add_particle_component");
}

void EffectDefinition::add_light_component(const LightComponent &lc) {
    print_line("add_light_component");
}

const Vector<EffectDefinition::ParticleComponent> &EffectDefinition::get_particles() const {}

const Vector<EffectDefinition::LightComponent> &EffectDefinition::get_lights() const {}
