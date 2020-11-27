import gym
from pdb import set_trace as T
from gym.utils.play import play
from griddly import GymWrapperFactory, gd

# This is what to use if you want to use OpenAI gym environments
wrapper = GymWrapperFactory()

wrapper.build_gym_from_yaml('nmmo', 'nmmo.yaml', level=0,
         player_observer_type=gd.ObserverType.VECTOR,
         global_observer_type=gd.ObserverType.VECTOR,
        )
#wrapper.build_gym_from_yaml('Spiders', 'Single-Player/Mini-Grid/Spiders.yaml', level=0)
# Create the Environment
env = gym.make(f'GDY-nmmo-v0')
#env = gym.make(f'GDY-Doggo-v0')
env.reset()
#play(env)
while True:
    act = env.action_space.sample()
    print(act)
    env.step(act)
    env.render(observer='global')
