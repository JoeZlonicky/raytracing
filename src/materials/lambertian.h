#pragma once

#include "../hittables/hit_record.h"
#include "../math/ray.h"
#include "../math/vector_3.h"
#include "material.h"

class Lambertian : public Material {
public:
	Lambertian(const Color& albedo);

private:
	Color _albedo;

	bool scatter(const Ray& r, const HitRecord& record, Color& attenuation_out, Ray& scattered_out) const override;
};