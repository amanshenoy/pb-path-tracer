#include <iostream>
#include <vector>

#include "objects.hpp"

bool Sphere::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    glm::vec3 ray_to_center = ray.origin() - center(ray.time()); 
    
    double a = glm::dot(ray.direction(), ray.direction());
    double half_b = glm::dot(ray_to_center, ray.direction()); 
    double c = glm::dot(ray_to_center, ray_to_center) - pow(radius, 2);
    double D = pow(half_b, 2) - a * c;
    if (D < 0) return false;
    
    double root = (-half_b - sqrt(D)) / a; 
    if (root < t_min || root > t_max){
        root = (-half_b + sqrt(D)) / a;
        if (root < t_min || t_max < root)
            return false;
    }
    rec.t = root;
    rec.p = ray.at(rec.t);
    
    glm::vec3 outward_normal = (rec.p - center(ray.time())) / static_cast<float>(radius);
    rec.set_face(ray, outward_normal); 
    get_sphere_uv(outward_normal, rec.u, rec.v);
    rec.mat_ptr = mat_ptr; 
    return true;    
}

bool Sphere::bounding_box(double time0, double time1, AABB& output_box) {
    AABB box_0(
        center(time0) - glm::vec3(radius),
        center(time0) + glm::vec3(radius)        
    ); 
    AABB box_1(
        center(time1) - glm::vec3(radius),
        center(time1) + glm::vec3(radius)        
    ); 

    output_box = surrounding_box(box_0, box_1); 
    return true; 
}

void Sphere::get_sphere_uv(const glm::vec3& p, double& u, double& v) {
    auto theta = acos(-p.y);
    auto phi = atan2(-p.z, p.x) + M_PI;

    u = phi / (2*M_PI);
    v = theta / M_PI;
}

glm::vec3 Sphere::center(double time) const {
    return center_t1 + static_cast<float>((time - t1) / (t2 - t1)) * (center_t2 - center_t1);
}

bool RectangleXY::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    double t = (k - ray.origin().z) / ray.direction().z;
    if (t < t_min || t > t_max)
        return false;
    double x = ray.origin().x + t * ray.direction().x; 
    double y = ray.origin().y + t * ray.direction().y;

    if (x < x1 || x > x2 || y < y1 || y > y2)
        return false; 
    
    rec.u = (x - x1) / (x2 - x1);
    rec.v = (y - y1) / (y2 - y1); 
    rec.t = t; 

    glm::vec3 outward_normal = glm::vec3(0.0f, 0.0f, 1.0f);
    rec.set_face(ray, outward_normal); 
    rec.mat_ptr = mat;
    rec.p = ray.at(t);
    return true;
}

bool RectangleXY::bounding_box(double time0, double time1, AABB& output_box) {
    output_box = AABB(glm::vec3(x1, y1, k - 0.0001), glm::vec3(x2, y2, k + 0.0001));
    return true;
}

bool RectangleYZ::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    double t = (k - ray.origin().x) / ray.direction().x;
    if (t < t_min || t > t_max)
        return false;
    double y = ray.origin().y + t * ray.direction().y; 
    double z = ray.origin().z + t * ray.direction().z;

    if (y < y1 || y > y2 || z < z1 || z > z2)
        return false; 
    
    rec.u = (y - y1) / (y2 - y1); 
    rec.v = (z - z1) / (z2 - z1);
    rec.t = t; 

    glm::vec3 outward_normal = glm::vec3(1.0f, 0.0f, 0.0f);
    rec.set_face(ray, outward_normal); 
    rec.mat_ptr = mat;
    rec.p = ray.at(t);
    return true;
}

bool RectangleYZ::bounding_box(double time0, double time1, AABB& output_box) {
    output_box = AABB(glm::vec3(y1, z1, k - 0.0001), glm::vec3(y2, z2, k + 0.0001));
    return true;
}


bool RectangleXZ::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    double t = (k - ray.origin().y) / ray.direction().y;
    if (t < t_min || t > t_max)
        return false;
    double x = ray.origin().x + t * ray.direction().x; 
    double z = ray.origin().z + t * ray.direction().z;

    if (x < x1 || x > x2 || z < z1 || z > z2)
        return false; 
    
    rec.u = (x - x1) / (x2 - x1);
    rec.v = (z - z1) / (z2 - z1); 
    rec.t = t; 

    glm::vec3 outward_normal = glm::vec3(0.0f, 1.0f, 0.0f);
    rec.set_face(ray, outward_normal); 
    rec.mat_ptr = mat;
    rec.p = ray.at(t);
    return true;
}

bool RectangleXZ::bounding_box(double time0, double time1, AABB& output_box) {
    output_box = AABB(glm::vec3(x1, z1, k - 0.0001), glm::vec3(x2, z2, k + 0.0001));
    return true;
}

bool Scene::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    hit_details temp_record; 
    bool hit_anything = false; 
    double closest = t_max;

    for (auto& object : objects){
        if (object -> hit(ray, t_min, closest, temp_record)){
            hit_anything = true;
            closest = temp_record.t; 
            rec = temp_record; 
        }
    }
    return hit_anything;
}

bool Scene::bounding_box(double time0, double time1, AABB& output_box) {
    if (objects.empty()) return false;

    AABB temp_box;
    bool first_box = true;
    for (const auto& object : objects) {
        if (!object->bounding_box(time0, time1, temp_box)) return false;
        output_box = first_box ? temp_box : surrounding_box(output_box, temp_box);
        first_box = false;
    }
    return true;
}

Box::Box(const glm::vec3& p1, const glm::vec3& p2, std::shared_ptr<Material> ptr) {
    box_min = p1;
    box_max = p2;

    faces.add(std::make_shared<RectangleXY>(p1.x , p1.y, p2.x  , p2.y , p2.z , ptr));
    faces.add(std::make_shared<RectangleXY>(p1.x , p1.y, p2.x  , p2.y , p1.z , ptr));

    faces.add(std::make_shared<RectangleXZ>(p1.x , p1.z,p2.x  , p2.z , p2.y , ptr));
    faces.add(std::make_shared<RectangleXZ>(p1.x , p1.z,p2.x  , p2.z , p1.y , ptr));

    faces.add(std::make_shared<RectangleYZ>(p1.y , p1.z,p2.y  , p2.z , p2.x , ptr));
    faces.add(std::make_shared<RectangleYZ>(p1.y , p1.z,p2.y  , p2.z , p1.x , ptr));
}

bool Box::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    return faces.hit(ray, t_min, t_max, rec);
}

bool Box::bounding_box(double time0, double time1, AABB& output_box) {
    output_box = AABB(box_min, box_max); 
    return true;    
}

bool Translate::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    Ray moved_ray(ray.origin() - translation, ray.direction(), ray.time());
    if (!ptr -> hit(moved_ray, t_min, t_max, rec))
        return false; 
    rec.p += translation;
    rec.set_face(moved_ray, rec.normal); 
    return true;
}

bool Translate::bounding_box(double time0, double time1, AABB& output_box) {
    if (!ptr -> bounding_box(time0, time1, output_box))
        return false; 
    output_box = AABB(output_box.min() + translation, output_box.max() + translation);
    
    return true; 
}

RotateY::RotateY(std::shared_ptr<Object> p, double angle) : ptr(p){
    auto radians = glm::radians(angle);
    sin_theta = sin(radians);
    cos_theta = cos(radians);
    hasbox = ptr->bounding_box(0, 1, bbox);
    double infinity = std::numeric_limits<double>::infinity();

    glm::vec3 min( infinity,  infinity,  infinity);
    glm::vec3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                auto x = i*bbox.max().x + (1-i)*bbox.min().x;
                auto y = j*bbox.max().y + (1-j)*bbox.min().y;
                auto z = k*bbox.max().z + (1-k)*bbox.min().z;

                auto newx =  cos_theta*x + sin_theta*z;
                auto newz = -sin_theta*x + cos_theta*z;

                glm::vec3 tester(newx, y, newz);

                for (int c = 0; c < 3; c++) {
                    min[c] = fmin(min[c], tester[c]);
                    max[c] = fmax(max[c], tester[c]);
                }
            }
        }
    }

    bbox = AABB(min, max);
}

bool RotateY::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    auto origin = ray.origin();
    auto direction = ray.direction();

    origin[0] = cos_theta*ray.origin()[0] - sin_theta*ray.origin()[2];
    origin[2] = sin_theta*ray.origin()[0] + cos_theta*ray.origin()[2];

    direction[0] = cos_theta*ray.direction()[0] - sin_theta*ray.direction()[2];
    direction[2] = sin_theta*ray.direction()[0] + cos_theta*ray.direction()[2];

    Ray rotated_r(origin, direction, ray.time());

    if (!ptr->hit(rotated_r, t_min, t_max, rec))
        return false;

    auto p = rec.p;
    auto normal = rec.normal;

    p[0] =  cos_theta*rec.p[0] + sin_theta*rec.p[2];
    p[2] = -sin_theta*rec.p[0] + cos_theta*rec.p[2];

    normal[0] =  cos_theta*rec.normal[0] + sin_theta*rec.normal[2];
    normal[2] = -sin_theta*rec.normal[0] + cos_theta*rec.normal[2];

    rec.p = p;
    rec.set_face(rotated_r, normal);

    return true;
}

bool RotateY::bounding_box(double time0, double time1, AABB& output_box) {
    output_box = bbox; 
    return hasbox;    
}

bool ConstantMedium::hit(const Ray& ray, double t_min, double t_max, hit_details& rec) {
    const bool enable_debug = false; 
    const bool debugging = enable_debug && random_double() < 0.00001; 

    hit_details rec1, rec2; 
    double infinity = std::numeric_limits<double>::infinity();


    if (!boundary->hit(ray, -infinity, infinity, rec1))
        return false;

    if (!boundary->hit(ray, rec1.t+0.0001, infinity, rec2))
        return false;

    if (debugging) std::cerr << "\nt_min=" << rec1.t << ", t_max=" << rec2.t << '\n';

    if (rec1.t < t_min) rec1.t = t_min;
    if (rec2.t > t_max) rec2.t = t_max;

    if (rec1.t >= rec2.t)
        return false;

    if (rec1.t < 0)
        rec1.t = 0;

    const auto ray_length = ray.direction().length();
    const auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
    const auto hit_distance = neg_inv_dens * log(random_double());

    if (hit_distance > distance_inside_boundary)
        return false;

    rec.t = rec1.t + hit_distance / ray_length;
    rec.p = ray.at(rec.t);

    if (debugging) {
        std::cerr << "hit_distance = " <<  hit_distance << '\n'
                  << "rec.t = " <<  rec.t << '\n' 
                  << "rec.p = " <<  rec.p.x << ',' 
                  << rec.p.y << ',' << rec.p.z << '\n';
    }

    rec.normal = glm::vec3(1,0,0);  // arbitrary
    rec.front_face = true;     // also arbitrary
    rec.mat_ptr = phase_fn;

    return true;
}

bool ConstantMedium::bounding_box(double time0, double time1, AABB& output_box) {
    return boundary -> bounding_box(time0, time1, output_box);
}