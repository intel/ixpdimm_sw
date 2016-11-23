#!/usr/bin/env python

from __future__ import print_function
import pywbem
import sys
import getpass


# support Python 2.x and 3.x for future compatibility
# In practice PyWBEM is only available for 2.x at this time
try:
    input = raw_input
except:
    pass

class IntelWbemTester(object):
    NAMESPACE_INTEL = 'root/intelwbem'
    NAMESPACE_INTEROP = 'root/interop'
    
    def __init__(self, namespace, host='localhost'):
        self.WBEM_HOST = host
        self.namespace = namespace
        self.connection = self.get_local_connection()
        self.assert_count = 0
    
    def get_local_connection(self):
        'Sets up a connection to a WBEM server.'
        print("Testing Intel WBEM requires admin permissions.")
        username = input("Admin username: ")
        password = getpass.getpass()
        
        return pywbem.WBEMConnection('http://' + self.WBEM_HOST, 
            creds=(username, password),
            default_namespace=self.namespace)
        

def start():
    tester = IntelWbemTester(IntelWbemTester.NAMESPACE_INTEL)
    processes = tester.connection.EnumerateInstances('Intel_BaseServer')
    process = processes[0]
    print("Value: ", process.items())
    process['LogMax'] = pywbem.Uint32(1000)
    print("Process: ", process.items())
    tester.connection.ModifyInstance(process, PropertyList=['LogMax'])
    print("Modifying done")
    processes1 = tester.connection.EnumerateInstances('Intel_BaseServer')
    process1 = processes1[0]
    print("Modified LogMax: ", process1.items()) 

def stop():
    tester.setup()


action = "start"
if len(sys.argv) > 1:
    action = sys.argv[1]
exec(action + "()")
