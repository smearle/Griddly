import os
import time
from pdb import set_trace as T

import gym
import numpy as np

#from gym.utils.play import play
import griddly
from griddly import GymWrapperFactory, gd
from griddly.nmmo.map_gen import MapGen
from griddly.nmmo.wrappers import NMMOWrapper
from griddly.util.rts_tools import InvalidMaskingRTSWrapper

if __name__ == '__main__':

    import copy

    for env in copy.deepcopy(gym.envs.registry.env_specs):
        if 'GDY' in env:
            print("Remove {} from registry".format(env))
            del gym.envs.registry.env_specs[env]

    # NB: The nmmo environment is designed to to render with an ISOMETRIC global observer only.
    wrapper = GymWrapperFactory()

    yaml_path = 'resources/games/nmmo.yaml'
    map_gen = MapGen()
    init_tiles, probs = map_gen.get_init_tiles(yaml_path)

    wrapper.build_gym_from_yaml('nmmo', yaml_path, level=0,
             player_observer_type=gd.ObserverType.VECTOR,
             global_observer_type=gd.ObserverType.ISOMETRIC,
            )
    # Create the Environment
    env = gym.make(f'GDY-nmmo-v0')
#   env = InvalidMaskingRTSWrapper(env)
    env = NMMOWrapper(env)

    def reset():
        level_string = map_gen.gen_map(init_tiles, probs)
        env.reset(level_id=None, level_string=level_string)
        env.render(observer='global')

    while True:
        reset()
        while True: pass
        while True:
            act = env.action_space.sample()
            obs, reward, done, infos = env.step(act)
            env.render(observer='global')

            if done:
                break
