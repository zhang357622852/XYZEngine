﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class ShaderAsset : UnmanagedResource
    {
        public ShaderAsset()
        {

        }
        public ShaderAsset(string path)
        {
            m_UnmanagedInstance = Constructor_Native(path);
        }

        ~ShaderAsset()
        {
            Destructor_Native(m_UnmanagedInstance);
        }
  
        internal ShaderAsset(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(string path);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
    }
}
