<!-- Vue 2.x  -->
<script>
  new Vue({
    el: '#example_vue',
    // Options...
      data() {
          return {
              count: 0,
              flags: {}
          };
      },
      created() {
          fetch('arches_flags.json')
              .then(response => response.json())
              .then(data => (this.flags = data))
              .catch(err => console.log(err));
      }
  });
</script>

<div id="example_vue">
    <button @click="count -= 1">-</button>
    {{ count }}
    <button @click="count += 1">+</button>
    <table>
        <tr v-for="flag in flags">
            <td>{{ flag[0] }}</td><td v-if="count>0">{{ flag[1] }}</td>
        </tr>
    </table>
</div>