states=[
    'Idle',
    'WaitMaster',
    'WaitPump',
    'WaitSlave',
    'WaitNextSlave',
    'SendCmd'
]

code=''

code+="""
class StateGestion:public State
{
   public:
      static StateMachine _machine;
      
"""

for s in states:
    tmp='      static State* st{s};\n'
    code+=tmp.replace('{s}',s)
    
code +="""
      StateGestion() :State(&StateGestion::_machine) {}
};

StateMachine StateGestion::_machine;
"""


code+='\n'
for s in states:
    cls="""
class State{s}:public StateGestion
{      
    void onRun() override
    {
    }    
};

"""

    code += cls.replace('{s}',s)

code += '\n'

for s in states:
    init='State * StateGestion::st{s} = new State{s}();\n'
    code += init.replace('{s}',s)

print(code)
    
