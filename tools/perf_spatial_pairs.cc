// Compare the production broad phase against an independent all-pairs oracle.
#include <Shared/Simulation.hh>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <vector>

void Simulation::on_tick() {}
void Simulation::post_tick() {}
namespace PetalTracker { void remove_petal(Simulation *, PetalID::T) {} }
namespace Map { void remove_mob(Simulation *, Entity const &) {} }
static std::vector<uint64_t> contacts;
static uint64_t calls = 0, ordered_hash = 0;
static bool touching(Entity const &a, Entity const &b) {
    float dx=a.get_x()-b.get_x(), dy=a.get_y()-b.get_y();
    float r=a.get_radius()+b.get_radius();
    return dx*dx+dy*dy <= r*r;
}
static void collect(Simulation *, Entity &a, Entity &b) {
    ++calls;
    if (!touching(a,b)) return;
    uint64_t key=(uint64_t)std::min(a.id.id,b.id.id)<<32 | std::max(a.id.id,b.id.id);
    contacts.push_back(key);
    ordered_hash=ordered_hash*1099511628211ull ^ key;
}
int main() {
    auto sim=std::make_unique<Simulation>();
    std::mt19937 rng(20261101);
    size_t total=0;
    double ms=0;
    for (unsigned scene=0;scene<5;++scene) {
        sim->reset();
        std::vector<EntityID> ids;
        // Sparse large mobs, mixed sizes, dense combat, oversized bodies,
        // and a cell overflow (the latter must retain the old fallback).
        unsigned count=scene==4?1100:1800;
        for(unsigned i=0;i<count;++i) {
            auto &e=sim->alloc_ent(); e.add_component(kPhysics);
            float radii[]={8,25,256,257,380,800,1400};
            float r=scene==0?300.f:radii[i%7];
            if(scene==3) r=std::max(300.f,r); // oversized fallback: big/big only
            if(scene==3 && i%100==0) r=5000.f;
            e.set_radius(scene==4?300.f:r);
            e.set_x(scene==4?4000.f:(float)(rng()%ARENA_WIDTH));
            e.set_y(scene==4?4000.f:(float)(rng()%ARENA_HEIGHT));
            if(scene==2) { e.set_x(3000.f+(float)(rng()%4000)); e.set_y(3000.f+(float)(rng()%4000)); }
            ids.push_back(e.id);
        }
        std::vector<uint64_t> expected;
        for(unsigned i=0;i<ids.size();++i)
            for(unsigned j=i+1;j<ids.size();++j)
                if(touching(sim->get_ent(ids[i]),sim->get_ent(ids[j])))
                    expected.push_back((uint64_t)ids[i].id<<32|ids[j].id);
        sim->spatial_hash.refresh(ARENA_WIDTH,ARENA_HEIGHT);
        for(auto id:ids)sim->spatial_hash.insert(sim->get_ent(id));
        contacts.clear(); calls=0; ordered_hash=0;
        auto start=std::chrono::steady_clock::now();
        sim->spatial_hash.collide(collect);
        double elapsed=std::chrono::duration<double,std::milli>(std::chrono::steady_clock::now()-start).count();
        ms+=elapsed;
        std::sort(contacts.begin(),contacts.end());
        if(expected!=contacts) { std::fprintf(stderr,"FAIL scene=%u expected=%zu actual=%zu\n",scene,expected.size(),contacts.size()); return 1; }
        total+=contacts.size();
        std::printf("scene=%u contacts=%zu calls=%llu order=%llu ms=%.3f PASS\n",scene,contacts.size(),(unsigned long long)calls,(unsigned long long)ordered_hash,elapsed);
    }
    std::printf("PASS %zu contacts (no misses or duplicates); %.3f ms\n",total,ms);
}
