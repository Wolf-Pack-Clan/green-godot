#ifndef EFFECT_DEF_H
#define EFFECT_DEF_H

#include "core/resource.h"

class EffectDefinition : public Resource {
	GDCLASS(EffectDefinition, Resource);

public:
	struct ParticleComponent {
		String particle_type;
		Dictionary properties;
	};
	struct LightComponent {
		String light_type;
		Dictionary properties;
	};

private:
	Vector<ParticleComponent> particles_components;
	Vector<LightComponent> light_components;

protected:
	static void _bind_methods();

	void add_particle_component(const ParticleComponent &pc);
	void add_light_component(const LightComponent &lc);

	const Vector<ParticleComponent> &get_particles() const;
	const Vector<LightComponent> &get_lights() const;
};

#endif // EFFECT_DEF_H
