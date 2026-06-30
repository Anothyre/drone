import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import path from 'path' // Das hier ist neu

export default defineConfig({
  plugins: [react()],
  resolve: {
    alias: {
      // Das hier sagt Vite: Wenn du ein @ siehst, schau im Ordner "src" nach
      '@': path.resolve(__dirname, './src'),
    },
  },
  server: {
    port: 5173,
    strictPort: true,
  }
})