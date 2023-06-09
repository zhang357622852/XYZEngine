﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.CompilerServices;

namespace XYZ
{
    public class Material : UnmanagedResource
    {
        public Material(Shader shader)
        {
            m_UnmanagedInstance = Constructor_Native(shader.m_UnmanagedInstance);
        }


        internal Material(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(IntPtr shaderInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
    }

    public class MaterialInstance : UnmanagedResource
    {
        public MaterialInstance(Material material)
        {
            m_UnmanagedInstance = Constructor_Native(material.m_UnmanagedInstance);
        }


        internal MaterialInstance(IntPtr instance)
        {
            m_UnmanagedInstance = instance;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern IntPtr Constructor_Native(IntPtr shaderInstance);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void Destructor_Native(IntPtr unmanagedInstance);
    }
}
